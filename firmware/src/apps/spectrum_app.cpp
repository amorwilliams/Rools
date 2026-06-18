#include "apps/spectrum_app.h"

namespace rools {

void SpectrumApp::Bind(Gfx* gfx)
{
    gfx_ = gfx;
}

void SpectrumApp::on_enter()
{
    // 进入 App 时清空采样环形缓冲，并同步分析器参数。
    ring_.Reset();
    analyzer_.SetGainDb(gain_db_);
    analyzer_.SetDecay(decay_);
}

void SpectrumApp::on_exit() {}

void SpectrumApp::audio_callback(const float* inL,
                                 const float* /*inR*/,
                                 float*       outL,
                                 float*       outR,
                                 size_t       n)
{
    // 频谱分析使用左声道做 mono 输入；音频直通到左右输出。
    for(size_t i = 0; i < n; ++i)
    {
        const float mono = inL[i];
        ring_.Push(mono);
        outL[i] = mono;
        outR[i] = mono;
    }
}

void SpectrumApp::ui_draw()
{
    if(!gfx_)
        return;

    // UI 线程批量取样，避免在 ISR 做 FFT。
    static float block[512];
    const size_t n = ring_.Pop(block, 512);
    if(n > 0)
        analyzer_.ProcessBlock(block, n);

    view_.Draw(*gfx_, analyzer_, peak_hold_, fullscreen_);
}

void SpectrumApp::on_enc(Enc enc, int delta)
{
    if(delta == 0)
        return;

    // Enc A 调当前参数；Enc B 选参数列。
    if(enc == Enc::A)
    {
        switch(param_idx_)
        {
        case 0:
            // Gain: -24..+24 dB
            gain_db_ += static_cast<float>(delta) * 1.5f;
            if(gain_db_ > 24.f)
                gain_db_ = 24.f;
            if(gain_db_ < -24.f)
                gain_db_ = -24.f;
            analyzer_.SetGainDb(gain_db_);
            break;
        case 1:
            // Decay: 0.1..0.95（越大峰值衰减越慢）
            decay_ += static_cast<float>(delta) * 0.02f;
            if(decay_ > 0.95f)
                decay_ = 0.95f;
            if(decay_ < 0.1f)
                decay_ = 0.1f;
            analyzer_.SetDecay(decay_);
            break;
        case 2:
            // FFT Size: 512 / 1024 切换
            analyzer_.SetFftSize(analyzer_.fft_size() == 512 ? 1024 : 512);
            break;
        default: break;
        }
    }
    else if(enc == Enc::B)
    {
        param_idx_ += delta;
        if(param_idx_ < 0)
            param_idx_ = 3;
        if(param_idx_ > 3)
            param_idx_ = 0;
    }
}

void SpectrumApp::on_enc_shift(Enc enc, int delta)
{
    if(delta == 0)
        return;

    if(enc == Enc::A)
    {
        switch(param_idx_)
        {
        case 0:
            gain_db_ += static_cast<float>(delta) * 0.5f;
            if(gain_db_ > 24.f)
                gain_db_ = 24.f;
            if(gain_db_ < -24.f)
                gain_db_ = -24.f;
            analyzer_.SetGainDb(gain_db_);
            break;
        case 1:
            decay_ += static_cast<float>(delta) * 0.005f;
            if(decay_ > 0.95f)
                decay_ = 0.95f;
            if(decay_ < 0.1f)
                decay_ = 0.1f;
            analyzer_.SetDecay(decay_);
            break;
        case 2:
            analyzer_.SetFftSize(analyzer_.fft_size() == 512 ? 1024 : 512);
            break;
        default: break;
        }
    }
    else if(enc == Enc::B)
    {
        fullscreen_ = !fullscreen_;
    }
}

void SpectrumApp::on_btn(Btn btn, bool pressed)
{
    if(btn != Btn::Center || !pressed)
        return;

    // 中键切换 Peak Hold。
    peak_hold_ = !peak_hold_;
}

const ParamMap* SpectrumApp::param_map() const
{
    static ParamMap map{"Gain", "Decay", "FFT Size", "Peak Hold"};
    return &map;
}

const char* SpectrumApp::current_a_hint() const
{
    static const char* hints[] = {
        "A: Gain",
        "A: Decay",
        "A: FFT Size",
        "A: none",
    };

    if(param_idx_ < 0 || param_idx_ > 3)
        return "A: -";
    return hints[param_idx_];
}

} // namespace rools
