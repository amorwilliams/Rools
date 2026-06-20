#include "app_shell.h"

#include "apps/app_registry.h"
#include "board/cv_reference.h"
#include "board/pins.h"
#include "daisy_seed.h"
#include "display/gfx.h"
#include "display/st7735.h"
#include "board/enc_debug.h"
#include "hid/encoder.h"
#include "hid/switch.h"
#include "ui/app_menu_view.h"
#include "ui/layout_view.h"

namespace rools {

using namespace daisy;

// 文件作用域外设；init() 前不可用
static DaisySeed   hw;
static AppShell*   shell_instance = nullptr; // ISR → process_audio 桥接
static St7735      display;
static Gfx         gfx(display);
static LayoutView  layout_view(gfx);
static Encoder     enc_a;
static Encoder     enc_b;
static Switch      btn_center;
static volatile int32_t enc_a_delta_accum  = 0;
static volatile int32_t enc_b_delta_accum  = 0;
static volatile uint8_t enc_a_press_events = 0; // bit0 rise, bit1 fall
static volatile uint8_t enc_b_press_events = 0; // bit0 rise, bit1 fall
static volatile uint8_t btn_events         = 0; // bit0 rise, bit1 fall
static constexpr uint8_t kEventRise        = 0x01;
static constexpr uint8_t kEventFall        = 0x02;
static AdcChannelConfig cv_adc_cfg[4];
static constexpr size_t kCvChannelCount = 4;

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
        enc_a_press_events |= kEventRise;
    if(enc_a.FallingEdge())
        enc_a_press_events |= kEventFall;
    if(enc_b.RisingEdge())
        enc_b_press_events |= kEventRise;
    if(enc_b.FallingEdge())
        enc_b_press_events |= kEventFall;
    if(btn_center.RisingEdge())
        btn_events |= kEventRise;
    if(btn_center.FallingEdge())
        btn_events |= kEventFall;

    if(shell_instance)
    {
        for(size_t i = 0; i < kCvChannelCount; ++i)
        {
            const float uni = hw.adc.GetFloat(static_cast<int>(i)); // 0..1 from ADC range
            const float cv  = CvAdcToNormalized(uni);
            shell_instance->columns[i].cv  = cv;
            shell_instance->columns[i].sum = cv;
        }
    }

    if(shell_instance)
        shell_instance->process_audio(in[0], in[1], out[0], out[1], size);
}

void AppShell::init()
{
    shell_instance = this;

    hw.Init();

    cv_adc_cfg[0].InitSingle(pins::kCv1Adc);
    cv_adc_cfg[1].InitSingle(pins::kCv2Adc);
    cv_adc_cfg[2].InitSingle(pins::kCv3Adc);
    cv_adc_cfg[3].InitSingle(pins::kCv4Adc);
    hw.adc.Init(cv_adc_cfg, 4);
    hw.adc.Start();

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
    bool     last_menu_open = false;
    layout_view.ResetCache();

    while(true)
    {
        int32_t da = 0;
        int32_t db = 0;
        uint8_t a_events = 0;
        uint8_t b_events = 0;
        uint8_t c_events = 0;

        __disable_irq();
        da                 = enc_a_delta_accum;
        db                 = enc_b_delta_accum;
        a_events           = enc_a_press_events;
        b_events           = enc_b_press_events;
        c_events           = btn_events;
        enc_a_delta_accum  = 0;
        enc_b_delta_accum  = 0;
        enc_a_press_events = 0;
        enc_b_press_events = 0;
        btn_events         = 0;
        __enable_irq();

        const uint32_t now = System::GetNow();
        bool           ui_dirty = false;

        const GestureFrameInput gesture_input{
            now, da, db, a_events, b_events, c_events, btn_center.Pressed(), enc_a.Pressed()};
        const GestureFrameResult gesture_result = gesture_controller_.Update(gesture_input);
        ui_dirty |= gesture_result.ui_dirty;

        const MenuUpdateInput menu_input{now,
                                         enc_a.Pressed(),
                                         AppRegistry::Count(),
                                         current_index_,
                                         gesture_result.events,
                                         gesture_result.count};
        const MenuResult menu_result = app_menu_view_.Update(menu_input);
        ui_dirty |= menu_result.ui_dirty;
        if(last_menu_open != menu_result.menu_open)
        {
            layout_view.ResetCache();
            ui_dirty       = true;
            last_menu_open = menu_result.menu_open;
        }
        if(menu_result.request_switch)
            request_app_switch(menu_result.switch_index);

        if(!menu_result.consumed)
            RouteInputToCurrentApp(gesture_result, ui_dirty);

        InputDebugUpdate(static_cast<int>(da),
                         static_cast<int>(db),
                         a_events,
                         b_events,
                         c_events,
                         enc_a.Pressed(),
                         enc_b.Pressed(),
                         btn_center.Pressed());

        const uint32_t refresh_interval = current_ ? current_->ui_refresh_interval_ms() : 33;
        if(ui_dirty || now - last_ui_ms >= refresh_interval)
        {
            if(gfx.IsBusy())
            {
                System::Delay(1);
                continue;
            }

            if(menu_result.menu_open)
            {
                app_menu_view_.Draw(gfx, AppRegistry::Count(), AppRegistry::Name);
                last_ui_ms = now;
            }
            else if(current_ && pending_index_ == kInvalidAppIndex)
            {
                layout_view.RenderAppFrame(current_, gesture_result.shift_active);
                last_ui_ms = now;
            }
        }
        System::Delay(1);
    }
}

void AppShell::RouteInputToCurrentApp(const GestureFrameResult& gesture_result, bool& ui_dirty)
{
    if(!current_)
        return;

    for(size_t i = 0; i < gesture_result.count; ++i)
    {
        const GestureEvent& e = gesture_result.events[i];
        switch(e.type)
        {
        case GestureType::EncTurnA: current_->on_enc(Enc::A, static_cast<int>(e.value)); break;
        case GestureType::EncTurnB: current_->on_enc(Enc::B, static_cast<int>(e.value)); break;
        case GestureType::ShiftEncTurnA:
            current_->on_enc_shift(Enc::A, static_cast<int>(e.value));
            break;
        case GestureType::ShiftEncTurnB:
            current_->on_enc_shift(Enc::B, static_cast<int>(e.value));
            break;
        case GestureType::EncPressA:
            if(gesture_result.shift_active)
                current_->on_enc_press_shift(Enc::A, true);
            else
                current_->on_enc_press(Enc::A, true);
            break;
        case GestureType::EncReleaseA:
            if(gesture_result.shift_active)
                current_->on_enc_press_shift(Enc::A, false);
            else
                current_->on_enc_press(Enc::A, false);
            break;
        case GestureType::EncPressB:
            if(gesture_result.shift_active)
                current_->on_enc_press_shift(Enc::B, true);
            else
                current_->on_enc_press(Enc::B, true);
            break;
        case GestureType::EncReleaseB:
            if(gesture_result.shift_active)
                current_->on_enc_press_shift(Enc::B, false);
            else
                current_->on_enc_press(Enc::B, false);
            break;
        case GestureType::CenterTap:
            if(gesture_result.shift_active)
            {
                current_->on_btn_shift(Btn::Center, true);
                current_->on_btn_shift(Btn::Center, false);
            }
            else
            {
                current_->on_btn(Btn::Center, true);
                current_->on_btn(Btn::Center, false);
            }
            break;
        case GestureType::ShiftEnter:
        case GestureType::ShiftExit:
            // Shift 状态由手势控制器维护，App 无需单独消费该事件。
            break;
        }
    }

    if(gesture_result.count > 0)
        ui_dirty = true;
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

    if(switch_phase_ == AppSwitchPhase::Idle && pending_index_ != kInvalidAppIndex)
        CommitAppSwitch();

    // TODO(M3): 按 kSwitchFadeMs / kAudioSampleRate 推进 fade_gain_
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

const ControlColumn* GetControlColumns()
{
    if(!shell_instance)
        return nullptr;
    return shell_instance->columns;
}

} // namespace rools
