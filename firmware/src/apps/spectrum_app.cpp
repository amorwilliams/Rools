#include "apps/spectrum_app.h"

#include <cmath>
#include <cstdio>

#include "display/theme.h"
#include "ui/layout_view.h"

namespace rools {

namespace {

size_t ClampCursorBin(size_t cursor_bin, size_t bins)
{
    if(bins == 0)
        return 0;
    return (cursor_bin >= bins) ? (bins - 1) : cursor_bin;
}

uint32_t BinToHz(size_t bin, size_t bins, size_t fft_size)
{
    if(bins == 0 || fft_size == 0)
        return 0;
    const size_t half = fft_size / 2;
    const size_t i0   = (bin * half) / bins;
    const float  hz   = (static_cast<float>(i0) * kAudioSampleRate) / static_cast<float>(fft_size);
    return static_cast<uint32_t>(hz + 0.5f);
}

} // namespace

void SpectrumApp::Bind(Gfx* gfx)
{
    gfx_ = gfx;
}

void SpectrumApp::on_enter()
{
    // 进入 App 时清空采样环形缓冲，并同步分析器参数。
    ring_.Reset();
    analyzer_.SetGainDb(gain_db_);
    analyzer_.SetDecay(peak_hold_decay_);
    freeze_latched_ = freeze_enabled_;
}

void SpectrumApp::on_exit() {}

void SpectrumApp::audio_callback(const float* inL,
                                 const float* inR,
                                 float*       outL,
                                 float*       outR,
                                 size_t       n)
{
    // 频谱分析使用 L/R 混合作为 mono 输入；避免只接右声道时出现 NO SIGNAL。
    float block_peak   = 0.f;
    float block_energy = 0.f;
    for(size_t i = 0; i < n; ++i)
    {
        const float mono = 0.5f * (inL[i] + inR[i]);
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
    if(n > 0 && !freeze_latched_)
        analyzer_.ProcessBlock(block, n);

    const size_t bins       = analyzer_.num_bins();
    const float* src_levels = peak_hold_ ? analyzer_.peaks() : analyzer_.bins();
    size_t       peak_bin   = 0;
    float        peak_level = 0.f;
    for(size_t i = 0; i < bins; ++i)
    {
        if(src_levels[i] > peak_level)
        {
            peak_level = src_levels[i];
            peak_bin   = i;
        }
    }
    cursor_bin_ = ClampCursorBin(cursor_bin_, bins);
    const float cursor_level = (bins > 0) ? src_levels[cursor_bin_] : 0.f;

    view_.Draw(*gfx_,
               analyzer_,
               peak_hold_,
               fullscreen_,
               layout.main_top,
               layout.main_bottom,
               freeze_latched_,
               peak_track_enabled_,
               peak_bin,
               cursor_enabled_,
               cursor_bin_);
    if(!fullscreen_)
    {
        char line[64];
        std::snprintf(line,
                      sizeof(line),
                      "P%uHz %.2f C%uHz %.2f",
                      static_cast<unsigned>(BinToHz(peak_bin, bins, analyzer_.fft_size())),
                      peak_level,
                      static_cast<unsigned>(BinToHz(cursor_bin_, bins, analyzer_.fft_size())),
                      cursor_level);
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
        int idx = static_cast<int>(focus_param_) + delta;
        const int count = static_cast<int>(FocusParam::Count);
        while(idx < 0)
            idx += count;
        focus_param_ = static_cast<FocusParam>(idx % count);
        return;
    }

    if(enc != Enc::B)
        return;

    const float gain_step  = fine_mode_ ? 0.5f : 1.5f;
    const float decay_step = fine_mode_ ? 0.005f : 0.02f;
    switch(focus_param_)
    {
    case FocusParam::PeakTrack:
        peak_track_enabled_ = !peak_track_enabled_;
        break;
    case FocusParam::Cursor:
    {
        const size_t bins = analyzer_.num_bins();
        if(bins > 0)
        {
            cursor_enabled_ = true;
            int next        = static_cast<int>(cursor_bin_) + delta;
            if(next < 0)
                next = 0;
            if(next >= static_cast<int>(bins))
                next = static_cast<int>(bins) - 1;
            cursor_bin_ = static_cast<size_t>(next);
        }
        break;
    }
    case FocusParam::Freeze:
        freeze_enabled_ = !freeze_enabled_;
        break;
    case FocusParam::Gain:
        // Gain: -24..+24 dB
        gain_db_ += static_cast<float>(delta) * gain_step;
        if(gain_db_ > 24.f)
            gain_db_ = 24.f;
        if(gain_db_ < -24.f)
            gain_db_ = -24.f;
        analyzer_.SetGainDb(gain_db_);
        break;
    case FocusParam::Decay:
        // Decay: 0.1..0.95（越大峰值衰减越慢）
        peak_hold_decay_ += static_cast<float>(delta) * decay_step;
        if(peak_hold_decay_ > 0.95f)
            peak_hold_decay_ = 0.95f;
        if(peak_hold_decay_ < 0.1f)
            peak_hold_decay_ = 0.1f;
        analyzer_.SetDecay(peak_hold_decay_);
        break;
    case FocusParam::FftSize:
        // FFT Size: 512 / 1024 切换
        analyzer_.SetFftSize(analyzer_.fft_size() == 512 ? 1024 : 512);
        cursor_bin_ = ClampCursorBin(cursor_bin_, analyzer_.num_bins());
        break;
    case FocusParam::PeakHold:
        peak_hold_ = !peak_hold_;
        break;
    default: break;
    }

    if(freeze_enabled_ != freeze_latched_)
    {
        if(!freeze_enabled_)
        {
            // 解除冻结时清理一次分析窗口，避免峰值/游标读数突然跳变。
            ring_.Reset();
            analyzer_.SetFftSize(analyzer_.fft_size());
            analyzer_.SetGainDb(gain_db_);
            analyzer_.SetDecay(peak_hold_decay_);
            cursor_bin_ = ClampCursorBin(cursor_bin_, analyzer_.num_bins());
        }
        freeze_latched_ = freeze_enabled_;
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

    freeze_enabled_ = !freeze_enabled_;
    if(freeze_enabled_ != freeze_latched_)
    {
        if(!freeze_enabled_)
        {
            ring_.Reset();
            analyzer_.SetFftSize(analyzer_.fft_size());
            analyzer_.SetGainDb(gain_db_);
            analyzer_.SetDecay(peak_hold_decay_);
            cursor_bin_ = ClampCursorBin(cursor_bin_, analyzer_.num_bins());
        }
        freeze_latched_ = freeze_enabled_;
    }
}

const ParamMap* SpectrumApp::param_map() const
{
    static ParamMap map{"PeakTrack", "Cursor", "Freeze", "Gain/Decay"};
    return &map;
}

const char* SpectrumApp::current_a_hint() const
{
    switch(focus_param_)
    {
    case FocusParam::PeakTrack: return "PeakTrack";
    case FocusParam::Cursor: return "Cursor";
    case FocusParam::Freeze: return "Freeze";
    case FocusParam::Gain: return "Gain";
    case FocusParam::Decay: return "Decay";
    case FocusParam::FftSize: return "FFT";
    case FocusParam::PeakHold: return "PeakHold";
    default: return "Param";
    }
}

const char* SpectrumApp::current_b_hint() const
{
    switch(focus_param_)
    {
    case FocusParam::PeakTrack:
        std::snprintf(b_hint_, sizeof(b_hint_), "%s", peak_track_enabled_ ? "ON" : "OFF");
        break;
    case FocusParam::Cursor:
        std::snprintf(b_hint_, sizeof(b_hint_), "%s %u", cursor_enabled_ ? "ON" : "OFF", static_cast<unsigned>(cursor_bin_));
        break;
    case FocusParam::Freeze:
        std::snprintf(b_hint_, sizeof(b_hint_), "%s", freeze_latched_ ? "ON" : "OFF");
        break;
    case FocusParam::Gain:
        std::snprintf(b_hint_, sizeof(b_hint_), "%+.1fdB", gain_db_);
        break;
    case FocusParam::Decay:
        std::snprintf(b_hint_, sizeof(b_hint_), "%.2f", peak_hold_decay_);
        break;
    case FocusParam::FftSize:
        std::snprintf(b_hint_, sizeof(b_hint_), "%u", static_cast<unsigned>(analyzer_.fft_size()));
        break;
    case FocusParam::PeakHold:
        std::snprintf(b_hint_, sizeof(b_hint_), "%s", peak_hold_ ? "ON" : "OFF");
        break;
    default: std::snprintf(b_hint_, sizeof(b_hint_), "-"); break;
    }
    return b_hint_;
}

} // namespace rools
