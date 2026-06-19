#include "apps/spectrum_app.h"

#include <cmath>
#include <cstdio>

#include "display/theme.h"
#include "ui/layout_view.h"

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
    float block_peak   = 0.f;
    float block_energy = 0.f;
    for(size_t i = 0; i < n; ++i)
    {
        const float mono = inL[i];
        const float abs_mono = std::fabs(mono);
        if(abs_mono > block_peak)
            block_peak = abs_mono;
        block_energy += mono * mono;
        ring_.Push(mono);
        outL[i] = mono;
        outR[i] = mono;
    }

    const float block_rms = std::sqrt(block_energy / static_cast<float>(n));
    in_peak_              = in_peak_ * 0.9f + block_peak * 0.1f;
    in_rms_               = in_rms_ * 0.9f + block_rms * 0.1f;
}

void SpectrumApp::ui_draw(const LayoutMetrics& layout)
{
    if(!gfx_)
        return;

    // UI 线程批量取样，避免在 ISR 做 FFT。
    static float block[512];
    const size_t n = ring_.Pop(block, 512);
    if(n > 0)
        analyzer_.ProcessBlock(block, n);

    view_.Draw(*gfx_, analyzer_, peak_hold_, fullscreen_, layout.main_top, layout.main_bottom);
    if(!fullscreen_)
    {
        char line[32];
        std::snprintf(line, sizeof(line), "IN p%.3f r%.3f", in_peak_, in_rms_);
        gfx_->DrawString(62, layout.main_top + 2, line, theme::kDefault.muted, theme::kDefault.bg);
    }
}

void SpectrumApp::on_enc(Enc enc, int delta)
{
    if(delta == 0)
        return;

    // Enc A 选参数；Enc B 改当前参数值。
    if(enc == Enc::A)
    {
        param_idx_ += delta;
        if(param_idx_ < 0)
            param_idx_ = 3;
        if(param_idx_ > 3)
            param_idx_ = 0;
        return;
    }

    if(enc != Enc::B)
        return;

    const float gain_step  = fine_mode_ ? 0.5f : 1.5f;
    const float decay_step = fine_mode_ ? 0.005f : 0.02f;
    switch(param_idx_)
    {
    case 0:
        // Gain: -24..+24 dB
        gain_db_ += static_cast<float>(delta) * gain_step;
        if(gain_db_ > 24.f)
            gain_db_ = 24.f;
        if(gain_db_ < -24.f)
            gain_db_ = -24.f;
        analyzer_.SetGainDb(gain_db_);
        break;
    case 1:
        // Decay: 0.1..0.95（越大峰值衰减越慢）
        decay_ += static_cast<float>(delta) * decay_step;
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

void SpectrumApp::on_enc_press(Enc enc, bool pressed)
{
    if(enc != Enc::B || !pressed)
        return;

    fine_mode_ = !fine_mode_;
}

void SpectrumApp::on_enc_shift(Enc enc, int delta)
{
    if(delta == 0)
        return;

    if(enc == Enc::B)
        fullscreen_ = !fullscreen_;
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
    switch(param_idx_)
    {
    case 0: return "Gain";
    case 1: return "Decay";
    case 2: return "FFT";
    case 3: return "Hold";
    default: return "Param";
    }
}

const char* SpectrumApp::current_b_hint() const
{
    return fine_mode_ ? "Fine" : "Coarse";
}

} // namespace rools
