#include "app_shell.h"

#include "apps/app_registry.h"
#include "apps/spectrum_app.h"
#include "board/pins.h"
#include "daisy_seed.h"
#include "display/gfx.h"
#include "display/st7735.h"
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

static void AudioCallback(AudioHandle::InputBuffer  in,
                          AudioHandle::OutputBuffer out,
                          size_t                    size)
{
    if(shell_instance)
        shell_instance->process_audio(in[0], in[1], out[0], out[1], size);
}

void AppShell::init()
{
    shell_instance = this;

    hw.Init();
    hw.StartAudio(AudioCallback);

    display.Init();

    enc_a.Init(pins::kEncA_A, pins::kEncA_B, pins::kEncA_Sw);
    enc_b.Init(pins::kEncB_A, pins::kEncB_B, pins::kEncB_Sw);
    btn_center.Init(pins::kBtnCenter);

    AppRegistry::BindUi(&gfx);

    load_app(0); // M1：boot 立即加载；运行时切换用 request_app_switch()
}

void AppShell::run_forever()
{
    uint32_t last_ui_ms = System::GetNow();

    while(true)
    {
        // --- 输入轮询（主线程）---
        enc_a.Debounce();
        enc_b.Debounce();
        btn_center.Debounce();

        PollEnc(enc_a, Enc::A);
        PollEnc(enc_b, Enc::B);

        if(btn_center.RisingEdge() && current_)
            current_->on_btn(Btn::Center, true);
        if(btn_center.FallingEdge() && current_)
            current_->on_btn(Btn::Center, false);

        // --- UI 刷新 ~30 fps ---
        const uint32_t now = System::GetNow();
        if(now - last_ui_ms >= 33)
        {
            last_ui_ms = now;
            if(current_)
                current_->ui_draw();
        }

        // TODO(M2): App 菜单 — Enc A 长按 → 列表 → request_app_switch(selected)

        System::Delay(1);
    }
}

void AppShell::PollEnc(Encoder& enc, Enc id)
{
    if(!current_)
        return;

    const int delta = enc.Increment();
    if(delta != 0)
        current_->on_enc(id, delta);

    if(enc.RisingEdge())
        current_->on_enc_press(id, true);
    if(enc.FallingEdge())
        current_->on_enc_press(id, false);
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
