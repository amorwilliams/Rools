#include "app_shell.h"

#include "apps/app_registry.h"
#include "apps/spectrum_app.h"
#include "board/pins.h"
#include "daisy_seed.h"
#include "display/gfx.h"
#include "display/st7735.h"
#include "board/enc_debug.h"
#include "hid/encoder.h"
#include "hid/switch.h"

namespace rools {

using namespace daisy;

// 文件作用域外设；init() 前不可用
static DaisySeed   hw;
static AppShell*   shell_instance = nullptr; // ISR → process_audio 桥接
static St7735      display;
static Gfx         gfx(display);
static Encoder     enc_a;
static Encoder     enc_b;
static Switch      btn_center;
static volatile int32_t enc_a_delta_accum  = 0;
static volatile int32_t enc_b_delta_accum  = 0;
static volatile uint8_t enc_a_press_events = 0; // bit0 rise, bit1 fall
static volatile uint8_t enc_b_press_events = 0; // bit0 rise, bit1 fall
static volatile uint8_t btn_events         = 0; // bit0 rise, bit1 fall

static void AudioCallback(AudioHandle::InputBuffer  in,
                          AudioHandle::OutputBuffer out,
                          size_t                    size)
{
    enc_a.Debounce();
    enc_b.Debounce();
    btn_center.Debounce();

    enc_a_delta_accum += enc_a.Increment();
    enc_b_delta_accum += enc_b.Increment();

    if(enc_a.RisingEdge())
        enc_a_press_events |= 0x01;
    if(enc_a.FallingEdge())
        enc_a_press_events |= 0x02;
    if(enc_b.RisingEdge())
        enc_b_press_events |= 0x01;
    if(enc_b.FallingEdge())
        enc_b_press_events |= 0x02;
    if(btn_center.RisingEdge())
        btn_events |= 0x01;
    if(btn_center.FallingEdge())
        btn_events |= 0x02;

    if(shell_instance)
        shell_instance->process_audio(in[0], in[1], out[0], out[1], size);
}

void AppShell::init()
{
    shell_instance = this;

    hw.Init();

    display.Init();

    enc_a.Init(pins::kEncA_A, pins::kEncA_B, pins::kEncA_Sw);
    enc_b.Init(pins::kEncB_A, pins::kEncB_B, pins::kEncB_Sw);
    btn_center.Init(pins::kBtnCenter);

    AppRegistry::BindUi(&gfx);

    load_app(0); // M1：boot 立即加载；运行时切换用 request_app_switch()

    hw.StartAudio(AudioCallback);
}

void AppShell::run_forever()
{
    uint32_t last_ui_ms = System::GetNow();

    while(true)
    {
        int32_t da = 0;
        int32_t db = 0;
        uint8_t a_events = 0;
        uint8_t b_events = 0;
        uint8_t c_events = 0;

        __disable_irq();
        da                = enc_a_delta_accum;
        db                = enc_b_delta_accum;
        a_events          = enc_a_press_events;
        b_events          = enc_b_press_events;
        c_events          = btn_events;
        enc_a_delta_accum = 0;
        enc_b_delta_accum = 0;
        enc_a_press_events = 0;
        enc_b_press_events = 0;
        btn_events         = 0;
        __enable_irq();

        bool ui_dirty = false;
        if(current_ && da != 0)
            current_->on_enc(Enc::A, static_cast<int>(da));
        if(current_ && db != 0)
            current_->on_enc(Enc::B, static_cast<int>(db));
        ui_dirty |= (da != 0 || db != 0);
        ui_dirty |= (a_events != 0 || b_events != 0 || c_events != 0);

        if((a_events & 0x01) && current_)
            current_->on_enc_press(Enc::A, true);
        if((a_events & 0x02) && current_)
            current_->on_enc_press(Enc::A, false);
        if((b_events & 0x01) && current_)
            current_->on_enc_press(Enc::B, true);
        if((b_events & 0x02) && current_)
            current_->on_enc_press(Enc::B, false);

        if((c_events & 0x01) && current_)
            current_->on_btn(Btn::Center, true);
        if((c_events & 0x02) && current_)
            current_->on_btn(Btn::Center, false);

        InputDebugUpdate(static_cast<int>(da),
                         static_cast<int>(db),
                         a_events,
                         b_events,
                         c_events,
                         enc_a.Pressed(),
                         enc_b.Pressed(),
                         btn_center.Pressed());

        // --- UI 刷新 ~30 fps；编码器变化时立即重绘 ---
        const uint32_t now = System::GetNow();
        if(ui_dirty || now - last_ui_ms >= 33)
        {
            last_ui_ms = now;
            if(current_)
                current_->ui_draw();
        }
        System::Delay(1);
    }
}

bool AppShell::load_app(size_t index)
{
    App* next = AppRegistry::Get(index);
    if(!next)
        return false;

    if(current_)
        current_->on_exit();

    current_       = next;
    current_index_ = index;
    pending_index_ = kInvalidAppIndex;
    switch_phase_  = AppSwitchPhase::Idle;
    fade_gain_     = 1.f;
    current_->on_enter();
    return true;
}

void AppShell::request_app_switch(size_t index)
{
    if(index >= AppRegistry::Count())
        return;

    if(index == current_index_ && switch_phase_ == AppSwitchPhase::Idle)
        return;

    pending_index_ = index;

    // TODO(M3): switch_phase_ = AppSwitchPhase::FadingOut;
    // 音频 ISR TickSwitchStateMachine() 驱动 fade → CommitAppSwitch()
}

size_t AppShell::app_count() const
{
    return AppRegistry::Count();
}

void AppShell::process_audio(const float* inL,
                             const float* inR,
                             float*       outL,
                             float*       outR,
                             size_t       n)
{
    audio_cb_internal(inL, inR, outL, outR, n);
}

void AppShell::TickSwitchStateMachine(size_t num_samples)
{
    (void)num_samples;

    // TODO(M3): 按 kSwitchFadeMs / kAudioSampleRate（board/audio_config.h）推进 fade_gain_
    //   FadingOut  → fade_gain_ → 0 → Switching → CommitAppSwitch()
    //   FadingIn   → fade_gain_ → 1 → Idle
    //
    // if(switch_phase_ == AppSwitchPhase::Idle && pending_index_ != kInvalidAppIndex)
    //     switch_phase_ = AppSwitchPhase::FadingOut;
}

void AppShell::CommitAppSwitch()
{
    if(pending_index_ == kInvalidAppIndex)
        return;

    App* next = AppRegistry::Get(pending_index_);
    if(!next)
    {
        pending_index_ = kInvalidAppIndex;
        switch_phase_  = AppSwitchPhase::Idle;
        return;
    }

    if(current_)
    {
        if(current_->uses_shared_dsp_memory())
            current_->on_release_shared_memory();
        current_->on_exit();
    }

    // TODO(M3): dsp_pool_.Reset();

    current_       = next;
    current_index_ = pending_index_;
    pending_index_ = kInvalidAppIndex;
    current_->on_enter();

    // TODO(M3): switch_phase_ = AppSwitchPhase::FadingIn;
    switch_phase_ = AppSwitchPhase::Idle;
    fade_gain_    = 1.f;
}

void AppShell::ApplyOutputFade(float* outL, float* outR, size_t n)
{
    if(fade_gain_ >= 1.f)
        return;

    // TODO(M3): 在 FadingOut/FadingIn 阶段对 outL/outR 乘 fade_gain_
    (void)outL;
    (void)outR;
    (void)n;
}

void AppShell::audio_cb_internal(const float* inL,
                                 const float* inR,
                                 float*       outL,
                                 float*       outR,
                                 size_t       n)
{
    TickSwitchStateMachine(n);

    if(current_)
    {
        current_->audio_callback(inL, inR, outL, outR, n);
        apply_mono_out(outL, outR, n);
        ApplyOutputFade(outL, outR, n);
        return;
    }

    // 无 App 时透明 passthrough
    for(size_t i = 0; i < n; i++)
    {
        outL[i] = inL[i];
        outR[i] = inR[i];
    }
    apply_mono_out(outL, outR, n);
    ApplyOutputFade(outL, outR, n);
}

void AppShell::apply_mono_out(float* outL, float* outR, size_t n)
{
    if(mono_mode != MonoMode::MonoOut && mono_mode != MonoMode::Auto)
        return;

    for(size_t i = 0; i < n; i++)
        outR[i] = outL[i];
}

} // namespace rools
