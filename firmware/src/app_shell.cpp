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

static DaisySeed   hw;
static AppShell*   shell_instance = nullptr;
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

    load_app(0);
}

void AppShell::run_forever()
{
    uint32_t last_ui_ms = System::GetNow();

    while(true)
    {
        enc_a.Debounce();
        enc_b.Debounce();
        btn_center.Debounce();

        PollEnc(enc_a, Enc::A);
        PollEnc(enc_b, Enc::B);

        if(btn_center.RisingEdge() && current_)
            current_->on_btn(Btn::Center, true);
        if(btn_center.FallingEdge() && current_)
            current_->on_btn(Btn::Center, false);

        const uint32_t now = System::GetNow();
        if(now - last_ui_ms >= 33)
        {
            last_ui_ms = now;
            if(current_)
                current_->ui_draw();
        }

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

    current_ = next;
    current_->on_enter();
    return true;
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

void AppShell::audio_cb_internal(const float* inL,
                                 const float* inR,
                                 float*       outL,
                                 float*       outR,
                                 size_t       n)
{
    if(current_)
    {
        current_->audio_callback(inL, inR, outL, outR, n);
        apply_mono_out(outL, outR, n);
        return;
    }

    for(size_t i = 0; i < n; i++)
    {
        outL[i] = inL[i];
        outR[i] = inR[i];
    }
    apply_mono_out(outL, outR, n);
}

void AppShell::apply_mono_out(float* outL, float* outR, size_t n)
{
    if(mono_mode != MonoMode::MonoOut && mono_mode != MonoMode::Auto)
        return;

    for(size_t i = 0; i < n; i++)
        outR[i] = outL[i];
}

} // namespace rools
