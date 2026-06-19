#include "apps/oscilloscope_app.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

#include "display/theme.h"
#include "sys/system.h"
#include "ui/layout_view.h"

namespace rools {

const float OscilloscopeApp::kTimeScalesMsPerDiv[kTimeScaleCount]
    = {0.05f, 0.1f,  0.2f,  0.5f,  1.0f,  2.0f,  5.0f,
       10.0f, 20.0f, 50.0f, 100.0f, 200.0f, 500.0f, 1000.0f};
const size_t OscilloscopeApp::kTimeDecimates[kTimeScaleCount]
    = {1, 1, 1, 1, 2, 4, 8, 16, 32, 64, 96, 128, 128, 128};
const float OscilloscopeApp::kVoltScalesPerDiv[kVoltScaleCount]
    = {0.01f, 0.02f, 0.05f, 0.1f, 0.2f, 0.5f, 1.0f, 2.0f, 5.0f, 10.0f};
static size_t s_render_write_idx = 0;

static void FormatTimePerDiv(char* dst, size_t dst_size, float ms_per_div)
{
    if(ms_per_div < 1.0f)
    {
        const int us = static_cast<int>(std::lround(ms_per_div * 1000.0f));
        std::snprintf(dst, dst_size, "%dus/div", us);
        return;
    }
    if(ms_per_div >= 1000.0f)
    {
        const int centis = static_cast<int>(std::lround(ms_per_div / 10.0f)); // 0.01s units
        const int spart  = centis / 100;
        const int frac   = std::abs(centis % 100);
        std::snprintf(dst, dst_size, "%d.%02ds/div", spart, frac);
        return;
    }
    const int tenths = static_cast<int>(std::lround(ms_per_div * 10.0f));
    const int mpart  = tenths / 10;
    const int frac   = std::abs(tenths % 10);
    std::snprintf(dst, dst_size, "%d.%dms/div", mpart, frac);
}

static void FormatVoltPerDiv(char* dst, size_t dst_size, float volt_per_div)
{
    if(volt_per_div < 1.0f)
    {
        const int mv = static_cast<int>(std::lround(volt_per_div * 1000.0f));
        std::snprintf(dst, dst_size, "%dmV/div", mv);
        return;
    }
    const int tenths = static_cast<int>(std::lround(volt_per_div * 10.0f));
    const int vpart  = tenths / 10;
    const int frac   = std::abs(tenths % 10);
    std::snprintf(dst, dst_size, "%d.%dV/div", vpart, frac);
}

static void FormatSignedVolts(char* dst, size_t dst_size, float volts, int decimals)
{
    int scale = 1;
    for(int i = 0; i < decimals; ++i)
        scale *= 10;
    const int scaled = static_cast<int>(std::lround(volts * static_cast<float>(scale)));
    const char sign  = (scaled >= 0) ? '+' : '-';
    const int  abs_v = std::abs(scaled);
    const int  iv    = abs_v / scale;
    const int  fv    = abs_v % scale;
    if(decimals == 2)
        std::snprintf(dst, dst_size, "%c%d.%02dV", sign, iv, fv);
    else if(decimals == 1)
        std::snprintf(dst, dst_size, "%c%d.%01dV", sign, iv, fv);
    else
        std::snprintf(dst, dst_size, "%c%dV", sign, iv);
}
static const OscilloscopeApp::InputSource kAudioFixedSlots[OscilloscopeApp::kMaxDisplayTraces]
    = {OscilloscopeApp::InputSource::Audio1,
       OscilloscopeApp::InputSource::Audio2,
       OscilloscopeApp::InputSource::Cv1,
       OscilloscopeApp::InputSource::Cv2};
static const OscilloscopeApp::InputSource kCvFixedSlots[OscilloscopeApp::kMaxDisplayTraces]
    = {OscilloscopeApp::InputSource::Cv1,
       OscilloscopeApp::InputSource::Cv2,
       OscilloscopeApp::InputSource::Cv3,
       OscilloscopeApp::InputSource::Cv4};

static const char* SourceName(OscilloscopeApp::InputSource src)
{
    switch(src)
    {
    case OscilloscopeApp::InputSource::Audio1: return "AIN1";
    case OscilloscopeApp::InputSource::Audio2: return "AIN2";
    case OscilloscopeApp::InputSource::Cv1: return "CV1";
    case OscilloscopeApp::InputSource::Cv2: return "CV2";
    case OscilloscopeApp::InputSource::Cv3: return "CV3";
    case OscilloscopeApp::InputSource::Cv4: return "CV4";
    default: return "--";
    }
}

static Color565 TraceColor(size_t slot)
{
    static const Color565 kTraceColors[OscilloscopeApp::kMaxDisplayTraces]
        = {color::Cyan, color::Magenta, color::Yellow, color::Green};
    return kTraceColors[slot % OscilloscopeApp::kMaxDisplayTraces];
}

static OscilloscopeApp::InputSource BoundSourceForSlot(OscilloscopeApp::PriorityMode mode, size_t slot)
{
    if(slot >= OscilloscopeApp::kMaxDisplayTraces)
        return OscilloscopeApp::InputSource::None;
    const OscilloscopeApp::InputSource* fixed_slots
        = (mode == OscilloscopeApp::PriorityMode::AudioFirst) ? kAudioFixedSlots : kCvFixedSlots;
    return fixed_slots[slot];
}

void OscilloscopeApp::Bind(Gfx* gfx)
{
    gfx_ = gfx;
}

void OscilloscopeApp::on_enter()
{
    write_idx_      = 0;
    sample_count_   = 0;
    decimate_phase_ = 0;
    trigger_hit_    = false;
    trigger_subsample_ = 0.f;
    use_peak_detect_ = false;
    trigger_stability_score_ = 0;
    draw_cols_      = 0;
    std::snprintf(focus_value_, sizeof(focus_value_), "-");
    std::snprintf(b_hint_, sizeof(b_hint_), "-");
    std::snprintf(top_hint_, sizeof(top_hint_), "Oscilloscope");
    for(size_t s = 0; s < kInputSourceCount; ++s)
    {
        trace_active_[s]        = false;
        trace_hold_until_ms_[s] = 0;
        for(size_t i = 0; i < kScopeBufSize; ++i)
            input_buffers_[s][i] = 0.f;
    }
    for(size_t slot = 0; slot < kMaxDisplayTraces; ++slot)
    {
        display_traces_[slot]           = InputSource::None;
        display_trace_has_signal_[slot] = false;
        trace_cfg_[slot].volt_idx = 6;
        for(size_t i = 0; i < kScreenCols; ++i)
        {
            draw_mean_[slot][i] = 0.f;
            draw_min_[slot][i]  = 0.f;
            draw_max_[slot][i]  = 0.f;
        }
        trace_dc_mean_[slot] = 0.f;
    }
    selected_trace_idx_ = 0;
    trigger_source_slot_ = 0;
    UpdateDisplayRouting();
}

void OscilloscopeApp::on_exit()
{
    if(gfx_)
        gfx_->SetLineQuality(Gfx::LineQuality::Fast);
}

void OscilloscopeApp::audio_callback(const float* inL,
                                     const float* inR,
                                     float*       outL,
                                     float*       outR,
                                     size_t       n)
{
    for(size_t i = 0; i < n; ++i)
    {
        const float l = inL[i];
        const float r = inR[i];
        outL[i]       = l;
        outR[i]       = r;

        if(hold_)
            continue;

        const size_t decimate = kTimeDecimates[time_idx_];
        if(++decimate_phase_ < decimate)
            continue;

        decimate_phase_ = 0;
        input_buffers_[static_cast<size_t>(InputSource::Audio1)][write_idx_] = l;
        input_buffers_[static_cast<size_t>(InputSource::Audio2)][write_idx_] = r;
        input_buffers_[static_cast<size_t>(InputSource::Cv1)][write_idx_]    = 0.f;
        input_buffers_[static_cast<size_t>(InputSource::Cv2)][write_idx_]    = 0.f;
        input_buffers_[static_cast<size_t>(InputSource::Cv3)][write_idx_]    = 0.f;
        input_buffers_[static_cast<size_t>(InputSource::Cv4)][write_idx_]    = 0.f;
        write_idx_ = (write_idx_ + 1) % kScopeBufSize;
        if(sample_count_ < kScopeBufSize)
            ++sample_count_;
    }
}

const float* OscilloscopeApp::SourceBuffer(InputSource src) const
{
    if(src == InputSource::None)
        return nullptr;
    return input_buffers_[static_cast<size_t>(src)];
}

bool OscilloscopeApp::IsCablePresentGPIO(InputSource, bool& known) const
{
    known = false;
    return false;
}

bool OscilloscopeApp::IsSignalActiveADC(InputSource src) const
{
    if(src == InputSource::None || sample_count_ < 16)
        return false;

    const float* src_buf = SourceBuffer(src);
    if(!src_buf)
        return false;

    const size_t window = std::min<size_t>(sample_count_, 128);
    float        peak   = 0.f;
    float        sum_sq = 0.f;
    for(size_t i = 0; i < window; ++i)
    {
        const size_t idx = (write_idx_ + kScopeBufSize - 1 - i) % kScopeBufSize;
        const float  v   = src_buf[idx];
        const float  a   = std::fabs(v);
        if(a > peak)
            peak = a;
        sum_sq += v * v;
    }
    const float rms = std::sqrt(sum_sq / static_cast<float>(window));
    return peak > 0.02f || rms > 0.01f;
}

bool OscilloscopeApp::IsActive(InputSource src, uint32_t now_ms)
{
    if(src == InputSource::None)
        return false;

    bool gpio_known = false;
    bool active     = false;
    if(IsCablePresentGPIO(src, gpio_known))
    {
        active = true;
    }
    else if(gpio_known)
    {
        active = false;
    }
    else
    {
        active = IsSignalActiveADC(src);
    }

    const size_t idx      = static_cast<size_t>(src);
    const float  hold_ms  = 120.0f;
    if(active)
    {
        trace_active_[idx]        = true;
        trace_hold_until_ms_[idx] = now_ms + static_cast<uint32_t>(hold_ms);
    }
    else if(trace_active_[idx] && now_ms < trace_hold_until_ms_[idx])
    {
        active = true;
    }
    else
    {
        trace_active_[idx] = false;
    }
    return active;
}

void OscilloscopeApp::UpdateDisplayRouting()
{
    const uint32_t now = daisy::System::GetNow();
    InputSource    next_src[kMaxDisplayTraces]
        = {InputSource::None, InputSource::None, InputSource::None, InputSource::None};
    bool next_has_signal[kMaxDisplayTraces] = {false, false, false, false};
    const InputSource* fixed_slots = (priority_mode_ == PriorityMode::AudioFirst) ? kAudioFixedSlots : kCvFixedSlots;

    for(size_t slot = 0; slot < kMaxDisplayTraces; ++slot)
    {
        const InputSource src = fixed_slots[slot];
        if(IsActive(src, now))
        {
            next_src[slot]        = src;
            next_has_signal[slot] = true;
        }
        else
        {
            next_src[slot]        = InputSource::None;
            next_has_signal[slot] = false;
        }
    }

    for(size_t i = 0; i < kMaxDisplayTraces; ++i)
    {
        display_traces_[i]           = next_src[i];
        display_trace_has_signal_[i] = next_has_signal[i];
    }

    if(selected_trace_idx_ >= kMaxDisplayTraces)
        selected_trace_idx_ = 0;

    trigger_source_slot_ %= kMaxDisplayTraces;
}

bool OscilloscopeApp::IsDisplaySlotVisible(size_t slot) const
{
    return slot < kMaxDisplayTraces && display_traces_[slot] != InputSource::None;
}

bool OscilloscopeApp::DetectAudioLike(const float* src, size_t total, size_t window)
{
    if(!src || total < 8 || window < 8)
        return false;

    const size_t end_linear = total;
    size_t       start      = (end_linear > window) ? (end_linear - window) : 0;
    if(start >= end_linear)
        return false;

    // 先用局部均值做中心线，避免非对称 CV/包络永远不过 0V。
    float  sum = 0.f;
    size_t cnt = 0;
    for(size_t l = start; l < end_linear; ++l)
    {
        const size_t idx = (s_render_write_idx + kScopeBufSize + l - total) % kScopeBufSize;
        sum += src[idx];
        ++cnt;
    }
    if(cnt == 0)
        return false;
    const float center = sum / static_cast<float>(cnt);
    const float dead   = 0.01f; // 死区，避免噪声抖动造成假过零。

    int prev_sign = 0;
    int crossings = 0;
    for(size_t l = start; l < end_linear; ++l)
    {
        const size_t idx = (s_render_write_idx + kScopeBufSize + l - total) % kScopeBufSize;
        const float  v   = src[idx] - center;
        int          s   = 0;
        if(v > dead)
            s = 1;
        else if(v < -dead)
            s = -1;
        if(s == 0)
            continue;
        if(prev_sign != 0 && s != prev_sign)
            ++crossings;
        prev_sign = s;
    }

    const float sample_rate = 48000.f / static_cast<float>(kTimeDecimates[time_idx_]);
    const float duration_s  = static_cast<float>(cnt) / sample_rate;
    if(duration_s <= 0.001f)
        return smart_audio_mode_;
    const float est_hz = static_cast<float>(crossings) / (2.0f * duration_s);

    // 迟滞阈值：高于 25Hz 认为音频；低于 10Hz 认为 LFO/CV；中间保持现状。
    if(est_hz > 25.f)
        return true;
    if(est_hz < 10.f)
        return false;
    return smart_audio_mode_;
}

bool OscilloscopeApp::ShouldAutoPeakMode() const
{
    // 长时基自动切到峰值包络绘制。
    return (time_idx_ >= 9);
}

float OscilloscopeApp::SampleLinearIndex(const float* src, size_t total, size_t linear_idx) const
{
    if(total == 0)
        return 0.f;
    if(linear_idx >= total)
        linear_idx = total - 1;
    const size_t idx = (s_render_write_idx + kScopeBufSize + linear_idx - total) % kScopeBufSize;
    return src[idx];
}

float OscilloscopeApp::InterpolateLinear(const float* src, size_t total, float linear_pos) const
{
    if(total < 2)
        return (total == 0) ? 0.f : SampleLinearIndex(src, total, 0);

    if(linear_pos < 0.f)
        linear_pos = 0.f;
    const float max_pos = static_cast<float>(total - 1);
    if(linear_pos > max_pos)
        linear_pos = max_pos;

    const size_t i0 = static_cast<size_t>(linear_pos);
    const size_t i1 = (i0 + 1 < total) ? (i0 + 1) : i0;
    const float  t  = linear_pos - static_cast<float>(i0);
    const float  v0 = SampleLinearIndex(src, total, i0);
    const float  v1 = SampleLinearIndex(src, total, i1);
    return v0 + (v1 - v0) * t;
}

float OscilloscopeApp::InterpolateCubic(const float* src, size_t total, float linear_pos) const
{
    if(total < 4)
        return InterpolateLinear(src, total, linear_pos);

    if(linear_pos < 0.f)
        linear_pos = 0.f;
    const float max_pos = static_cast<float>(total - 1);
    if(linear_pos > max_pos)
        linear_pos = max_pos;

    const int   i1 = static_cast<int>(std::floor(linear_pos));
    const float t  = linear_pos - static_cast<float>(i1);
    int         i0 = i1 - 1;
    int         i2 = i1 + 1;
    int         i3 = i1 + 2;
    if(i0 < 0)
        i0 = 0;
    if(i2 >= static_cast<int>(total))
        i2 = static_cast<int>(total) - 1;
    if(i3 >= static_cast<int>(total))
        i3 = static_cast<int>(total) - 1;

    const float p0 = SampleLinearIndex(src, total, static_cast<size_t>(i0));
    const float p1 = SampleLinearIndex(src, total, static_cast<size_t>(i1));
    const float p2 = SampleLinearIndex(src, total, static_cast<size_t>(i2));
    const float p3 = SampleLinearIndex(src, total, static_cast<size_t>(i3));

    const float a0 = -0.5f * p0 + 1.5f * p1 - 1.5f * p2 + 0.5f * p3;
    const float a1 = p0 - 2.5f * p1 + 2.0f * p2 - 0.5f * p3;
    const float a2 = -0.5f * p0 + 0.5f * p2;
    const float a3 = p1;
    return ((a0 * t + a1) * t + a2) * t + a3;
}

float OscilloscopeApp::InterpolateSample(const float* src, size_t total, float linear_pos) const
{
    return InterpolateLinear(src, total, linear_pos);
}

bool OscilloscopeApp::FindTriggerStart(const float* src,
                                       size_t       total,
                                       size_t       window,
                                       size_t&      start,
                                       float&       subsample) const
{
    subsample = 0.f;
    if(total < window + 2 || window < 2)
        return false;

    const float  level = trigger_level_ / 5.0f;
    const float  hys   = 0.02f; // ~100mV hysteresis around level.
    const float  low   = level - hys;
    const float  high  = level + hys;
    bool         found         = false;
    size_t       best_start    = 0;
    float        best_subsample = 0.f;
    bool         armed = false;

    for(size_t i = 1; i < total; ++i)
    {
        const float prev = SampleLinearIndex(src, total, i - 1);
        const float curr = SampleLinearIndex(src, total, i);
        bool        triggered = false;
        float       frac      = 0.f;
        if(trigger_mode_ != TriggerMode::Fall)
        {
            if(prev < low)
                armed = true;
            if(armed && curr >= high)
            {
                triggered = true;
                const float denom = curr - prev;
                frac = (denom != 0.f) ? (level - prev) / denom : 0.f;
                armed = false;
            }
        }
        else
        {
            if(prev > high)
                armed = true;
            if(armed && curr <= low)
            {
                triggered = true;
                const float denom = curr - prev;
                frac = (denom != 0.f) ? (level - prev) / denom : 0.f;
                armed = false;
            }
        }

        if(frac < 0.f)
            frac = 0.f;
        if(frac > 1.f)
            frac = 1.f;

        if(triggered)
        {
            float start_f = (static_cast<float>(i) - 1.f + frac) - static_cast<float>(window) * 0.5f;
            if(start_f >= 0.f && start_f + static_cast<float>(window) <= static_cast<float>(total))
            {
                found          = true;
                best_start     = static_cast<size_t>(start_f);
                best_subsample = start_f - static_cast<float>(best_start);
            }
        }
    }

    if(!found)
        return false;

    start     = best_start;
    subsample = best_subsample;
    return true;
}

void OscilloscopeApp::CaptureWindow()
{
    const auto ClearDrawBuffers = [this]() {
        for(size_t slot = 0; slot < kMaxDisplayTraces; ++slot)
        {
            for(size_t x = 0; x < kScreenCols; ++x)
            {
                draw_mean_[slot][x] = 0.f;
                draw_min_[slot][x]  = 0.f;
                draw_max_[slot][x]  = 0.f;
            }
            trace_dc_mean_[slot] = 0.f;
        }
        draw_cols_ = 0;
    };

    if(sample_count_ < 2)
    {
        ClearDrawBuffers();
        return;
    }

    s_render_write_idx = write_idx_;

    const float  sample_rate = 48000.f / static_cast<float>(kTimeDecimates[time_idx_]);
    const float  ms_per_div  = kTimeScalesMsPerDiv[time_idx_];
    const float  total_ms    = ms_per_div * 12.0f;
    const size_t desired     = static_cast<size_t>((total_ms * sample_rate) / 1000.0f);
    const size_t window      = (desired < 64) ? 64 : (desired > kScopeBufSize ? kScopeBufSize : desired);

    const size_t total      = sample_count_;
    const size_t end_linear = total;
    size_t       start_linear = (end_linear > window) ? (end_linear - window) : 0;
    const float  mid_ms_per_div = kTimeScalesMsPerDiv[time_idx_];

    const InputSource trigger_src = IsDisplaySlotVisible(trigger_source_slot_) ? display_traces_[trigger_source_slot_]
                                                                               : InputSource::None;
    const float* trig_buf = SourceBuffer(trigger_src);

    const bool norm_like_mode = (trigger_mode_ != TriggerMode::Auto);
    bool       use_roll_mode  = false;
    if(mid_ms_per_div >= 100.0f)
        use_roll_mode = true;
    else if(mid_ms_per_div > 20.0f)
        use_roll_mode = (trigger_stability_score_ < 0);

    trigger_subsample_ = 0.f;
    trigger_hit_       = false;
    if(!use_roll_mode)
    {
        trigger_hit_ = trig_buf && FindTriggerStart(trig_buf, end_linear, window, start_linear, trigger_subsample_);
        if(!trigger_hit_)
        {
            if(norm_like_mode)
            {
                ClearDrawBuffers();
                return;
            }
            trigger_subsample_ = 0.f;
        }
    }
    else if(mid_ms_per_div > 20.0f
            && mid_ms_per_div < 100.0f
            && trig_buf)
    {
        size_t probe_start = start_linear;
        float  probe_sub   = 0.f;
        trigger_hit_       = FindTriggerStart(trig_buf, end_linear, window, probe_start, probe_sub);
    }

    if(mid_ms_per_div > 20.0f && mid_ms_per_div < 100.0f)
    {
        if(trigger_hit_)
        {
            if(trigger_stability_score_ < 8)
                ++trigger_stability_score_;
        }
        else
        {
            if(trigger_stability_score_ > -8)
                --trigger_stability_score_;
        }
    }

    draw_cols_ = kScreenCols;
    const float start_pos = static_cast<float>(start_linear) + trigger_subsample_;
    const float step      = static_cast<float>(window) / static_cast<float>(draw_cols_);

    const float samples_per_px = static_cast<float>(window) / static_cast<float>(kScreenCols);
    const bool  auto_peak      = ShouldAutoPeakMode() || (samples_per_px >= 2.0f);
    use_peak_detect_           = auto_peak;
    if(!use_peak_detect_)
    {
        for(size_t slot = 0; slot < kMaxDisplayTraces; ++slot)
        {
            const float* src = SourceBuffer(display_traces_[slot]);
            if(!src)
                continue;
            for(size_t x = 0; x < draw_cols_; ++x)
            {
                const float center = start_pos + (static_cast<float>(x) + 0.5f) * step;
                const float v      = InterpolateSample(src, total, center);
                draw_mean_[slot][x] = v;
                draw_min_[slot][x]  = v;
                draw_max_[slot][x]  = v;
            }
        }
        return;
    }

    for(size_t slot = 0; slot < kMaxDisplayTraces; ++slot)
    {
        const float* src = SourceBuffer(display_traces_[slot]);
        if(!src)
            continue;
        for(size_t x = 0; x < draw_cols_; ++x)
        {
            size_t l0 = static_cast<size_t>(start_pos + static_cast<float>(x) * step);
            size_t l1 = static_cast<size_t>(start_pos + static_cast<float>(x + 1) * step);
            if(l1 <= l0)
                l1 = l0 + 1;
            if(l1 > end_linear)
                l1 = end_linear;

            float  min_v = 1e9f;
            float  max_v = -1e9f;
            float  sum   = 0.f;
            size_t cnt   = 0;
            for(size_t l = l0; l < l1; ++l)
            {
                const size_t idx = (s_render_write_idx + kScopeBufSize + l - total) % kScopeBufSize;
                const float  v   = src[idx];
                if(v < min_v)
                    min_v = v;
                if(v > max_v)
                    max_v = v;
                sum += v;
                ++cnt;
            }
            if(cnt == 0)
            {
                min_v = 0.f;
                max_v = 0.f;
            }
            draw_min_[slot][x]  = min_v;
            draw_max_[slot][x]  = max_v;
            draw_mean_[slot][x] = (cnt == 0) ? 0.f : (sum / static_cast<float>(cnt));
        }
    }

    if(draw_cols_ > 0)
    {
        const float inv_cols = 1.0f / static_cast<float>(draw_cols_);
        for(size_t slot = 0; slot < kMaxDisplayTraces; ++slot)
        {
            float sum = 0.f;
            for(size_t x = 0; x < draw_cols_; ++x)
                sum += draw_mean_[slot][x];
            trace_dc_mean_[slot] = sum * inv_cols * 5.0f; // normalized sample -> volts
        }
    }
}

void OscilloscopeApp::BuildFocusValueText()
{
    switch(focus_param_)
    {
    case FocusParam::TimeScale:
        FormatTimePerDiv(focus_value_, sizeof(focus_value_), kTimeScalesMsPerDiv[time_idx_]);
        break;
    case FocusParam::VoltScale:
        FormatVoltPerDiv(focus_value_,
                         sizeof(focus_value_),
                         kVoltScalesPerDiv[trace_cfg_[selected_trace_idx_].volt_idx]);
        break;
    case FocusParam::TriggerSource:
        std::snprintf(focus_value_, sizeof(focus_value_), "TRG %d", static_cast<int>(trigger_source_slot_ + 1));
        break;
    case FocusParam::TriggerMode:
        std::snprintf(focus_value_,
                      sizeof(focus_value_),
                      "%s",
                      trigger_mode_ == TriggerMode::Auto   ? "AUTO"
                      : trigger_mode_ == TriggerMode::Rise ? "RISE"
                                                           : "FALL");
        break;
    case FocusParam::TriggerLevel:
        FormatSignedVolts(focus_value_, sizeof(focus_value_), trigger_level_, 2);
        break;
    case FocusParam::Priority:
        std::snprintf(focus_value_,
                      sizeof(focus_value_),
                      "%s",
                      priority_mode_ == PriorityMode::AudioFirst ? "AUDIO FIRST" : "CV FIRST");
        break;
    default:
        std::snprintf(focus_value_, sizeof(focus_value_), "-");
        break;
    }
    std::snprintf(b_hint_, sizeof(b_hint_), "%.16s", focus_value_);
}

void OscilloscopeApp::ui_draw(const LayoutMetrics& layout)
{
    if(!gfx_)
        return;

    UpdateDisplayRouting();
    const size_t visible_count = IsDisplaySlotVisible(3) ? 4 : IsDisplaySlotVisible(2) ? 3 : IsDisplaySlotVisible(1) ? 2
                                                                                         : IsDisplaySlotVisible(0) ? 1 : 0;
    const bool multi_trace = visible_count > 1;
    gfx_->SetLineQuality((aa_enabled_ && !multi_trace) ? Gfx::LineQuality::AntiAliased : Gfx::LineQuality::Fast);
    CaptureWindow();

    char route_label[24];
    char time_label[12];
    char volt_label[12];
    char time_full[24];
    char volt_full[24];
    const InputSource selected_bound_src = BoundSourceForSlot(priority_mode_, selected_trace_idx_);
    std::snprintf(route_label, sizeof(route_label), "SEL %s", SourceName(selected_bound_src));
    if(kTimeScalesMsPerDiv[time_idx_] < 1.0f)
        std::snprintf(time_full, sizeof(time_full), "%dus", static_cast<int>(std::lround(kTimeScalesMsPerDiv[time_idx_] * 1000.0f)));
    else if(kTimeScalesMsPerDiv[time_idx_] >= 1000.0f)
    {
        const int centis = static_cast<int>(std::lround(kTimeScalesMsPerDiv[time_idx_] / 10.0f));
        const int spart  = centis / 100;
        const int frac   = std::abs(centis % 100);
        std::snprintf(time_full, sizeof(time_full), "%d.%02ds", spart, frac);
    }
    else
    {
        const int tenths = static_cast<int>(std::lround(kTimeScalesMsPerDiv[time_idx_] * 10.0f));
        const int mpart  = tenths / 10;
        const int frac   = std::abs(tenths % 10);
        std::snprintf(time_full, sizeof(time_full), "%d.%dms", mpart, frac);
    }
    std::snprintf(time_label, sizeof(time_label), "%.11s", time_full);
    if(kVoltScalesPerDiv[trace_cfg_[selected_trace_idx_].volt_idx] < 1.0f)
        std::snprintf(volt_full,
                      sizeof(volt_full),
                      "%dmV",
                      static_cast<int>(std::lround(kVoltScalesPerDiv[trace_cfg_[selected_trace_idx_].volt_idx] * 1000.0f)));
    else
    {
        const int tenths = static_cast<int>(std::lround(kVoltScalesPerDiv[trace_cfg_[selected_trace_idx_].volt_idx] * 10.0f));
        const int vpart  = tenths / 10;
        const int frac   = std::abs(tenths % 10);
        std::snprintf(volt_full, sizeof(volt_full), "%d.%dV", vpart, frac);
    }
    std::snprintf(volt_label, sizeof(volt_label), "%.11s", volt_full);
    BuildFocusValueText();

    const OscilloscopeViewState state{
        !hold_,
        hold_,
        fine_mode_,
        route_label,
        time_label,
        volt_label,
        trigger_mode_ == TriggerMode::Auto   ? "AUTO"
        : trigger_mode_ == TriggerMode::Rise ? "RISE"
                                             : "FALL",
        trigger_mode_ == TriggerMode::Fall ? "FALL" : "RISE",
        use_peak_detect_ ? "PEAK" : "SAMPLE",
        kVoltScalesPerDiv[trace_cfg_[trigger_source_slot_].volt_idx],
        trigger_level_,
        use_peak_detect_,
        layout.main_top,
        layout.main_bottom,
    };
    OscilloscopeTraceView traces[kMaxDisplayTraces];
    char                  trace_labels[kMaxDisplayTraces][12];
    for(size_t slot = 0; slot < kMaxDisplayTraces; ++slot)
    {
        const InputSource bound_src = BoundSourceForSlot(priority_mode_, slot);
        std::snprintf(trace_labels[slot], sizeof(trace_labels[slot]), "T%d:%s", static_cast<int>(slot) + 1, SourceName(bound_src));
        traces[slot].label        = trace_labels[slot];
        traces[slot].mean_samples = draw_mean_[slot];
        traces[slot].min_samples  = draw_min_[slot];
        traces[slot].max_samples  = draw_max_[slot];
        traces[slot].volt_per_div = kVoltScalesPerDiv[trace_cfg_[slot].volt_idx];
        traces[slot].color        = TraceColor(slot);
        traces[slot].has_signal   = display_trace_has_signal_[slot];
        traces[slot].visible      = IsDisplaySlotVisible(slot);
        traces[slot].selected     = (slot == selected_trace_idx_);
    }
    view_.Draw(*gfx_, state, traces, kMaxDisplayTraces, draw_cols_, trigger_hit_);
}

void OscilloscopeApp::AdvanceFocus(int delta)
{
    if(delta == 0)
        return;
    int idx = static_cast<int>(focus_param_) + delta;
    const int count = static_cast<int>(FocusParam::Count);
    for(int guard = 0; guard < count; ++guard)
    {
        while(idx < 0)
            idx += count;
        const FocusParam candidate = static_cast<FocusParam>(idx % count);
        const bool hidden_in_auto
            = (trigger_mode_ == TriggerMode::Auto) && (candidate == FocusParam::TriggerLevel);
        if(!hidden_in_auto)
        {
            focus_param_ = candidate;
            return;
        }
        idx += (delta > 0) ? 1 : -1;
    }
}

void OscilloscopeApp::AdjustFocusedParam(int delta)
{
    if(delta == 0)
        return;
    const int step = fine_mode_ ? ((delta > 0) ? 1 : -1) : delta;
    switch(focus_param_)
    {
    case FocusParam::TimeScale:
        time_idx_ += step;
        if(time_idx_ < 0)
            time_idx_ = 0;
        if(time_idx_ >= static_cast<int>(kTimeScaleCount))
            time_idx_ = static_cast<int>(kTimeScaleCount) - 1;
        break;
    case FocusParam::VoltScale:
        trace_cfg_[selected_trace_idx_].volt_idx += step;
        if(trace_cfg_[selected_trace_idx_].volt_idx < 0)
            trace_cfg_[selected_trace_idx_].volt_idx = 0;
        if(trace_cfg_[selected_trace_idx_].volt_idx >= static_cast<int>(kVoltScaleCount))
            trace_cfg_[selected_trace_idx_].volt_idx = static_cast<int>(kVoltScaleCount) - 1;
        break;
    case FocusParam::TriggerSource:
        trigger_source_slot_ = static_cast<size_t>(
            (static_cast<int>(trigger_source_slot_) + step + static_cast<int>(kMaxDisplayTraces) * 8)
            % static_cast<int>(kMaxDisplayTraces));
        UpdateDisplayRouting();
        break;
    case FocusParam::TriggerMode:
    {
        int mode = static_cast<int>(trigger_mode_);
        mode += (step > 0) ? 1 : -1;
        while(mode < 0)
            mode += 3;
        trigger_mode_ = static_cast<TriggerMode>(mode % 3);
        if(trigger_mode_ == TriggerMode::Auto && focus_param_ == FocusParam::TriggerLevel)
            focus_param_ = FocusParam::TriggerMode;
        break;
    }
    case FocusParam::TriggerLevel:
        trigger_level_ += static_cast<float>(step) * (fine_mode_ ? 0.01f : 0.1f);
        if(trigger_level_ > 5.f)
            trigger_level_ = 5.f;
        if(trigger_level_ < -5.f)
            trigger_level_ = -5.f;
        break;
    case FocusParam::Priority:
        priority_mode_ = (step > 0) ? PriorityMode::CvFirst : PriorityMode::AudioFirst;
        UpdateDisplayRouting();
        break;
    default: break;
    }
}

void OscilloscopeApp::SelectTrace(int dir)
{
    if(dir == 0)
        return;
    if(dir > 0)
        selected_trace_idx_ = (selected_trace_idx_ + 1) % kMaxDisplayTraces;
    else
        selected_trace_idx_ = (selected_trace_idx_ + kMaxDisplayTraces - 1) % kMaxDisplayTraces;
}

void OscilloscopeApp::on_enc(Enc enc, int delta)
{
    if(delta == 0)
        return;
    if(enc == Enc::A)
        AdvanceFocus(delta);
    else if(enc == Enc::B)
        AdjustFocusedParam(delta);
}

void OscilloscopeApp::on_enc_press(Enc enc, bool pressed)
{
    if(!pressed)
        return;
    if(enc == Enc::A)
        SelectTrace(-1);
    else if(enc == Enc::B)
        SelectTrace(1);
}

void OscilloscopeApp::on_enc_press_shift(Enc enc, bool pressed)
{
    if(enc == Enc::B && pressed)
        fine_mode_ = !fine_mode_;
}

void OscilloscopeApp::on_enc_shift(Enc enc, int delta)
{
    if(enc == Enc::B && delta != 0)
        hold_ = !hold_;
}

void OscilloscopeApp::on_btn(Btn btn, bool pressed)
{
    if(btn == Btn::Center && pressed)
        hold_ = !hold_;
}

const ParamMap* OscilloscopeApp::param_map() const
{
    static ParamMap map{"Time", "Volt", "Trigger", "Hold"};
    return &map;
}

const char* OscilloscopeApp::current_a_hint() const
{
    switch(focus_param_)
    {
    case FocusParam::TimeScale: return "Time";
    case FocusParam::VoltScale: return "Volt";
    case FocusParam::TriggerSource: return "TrigSrc";
    case FocusParam::TriggerMode: return "Trig";
    case FocusParam::TriggerLevel: return "Level";
    case FocusParam::Priority: return "Priority";
    default: return "Param";
    }
}

const char* OscilloscopeApp::current_b_hint() const
{
    return b_hint_;
}

bool OscilloscopeApp::current_top_hint(Gfx& gfx, const char*& out_text) const
{
    const auto& t = theme::kDefault;
    gfx.FillRect(0, 0, Gfx::kWidth, LayoutView::kTopBarHeight, t.bg);

    const size_t slot = selected_trace_idx_ % kMaxDisplayTraces;
    const bool   has_signal = display_trace_has_signal_[slot];
    const Color565 trace_color = TraceColor(slot);
    char status_text[16];
    char mean_text[8];
    const char* trig_short = trigger_mode_ == TriggerMode::Auto   ? "A"
                             : trigger_mode_ == TriggerMode::Rise ? "R"
                                                                   : "F";
    const InputSource bound_src = BoundSourceForSlot(priority_mode_, slot);
    std::snprintf(top_hint_, sizeof(top_hint_), "%s", SourceName(bound_src));
    {
        const int scaled = static_cast<int>(std::lround(trace_dc_mean_[slot] * 10.0f));
        const char sign  = (scaled >= 0) ? '+' : '-';
        const int  abs_v = std::abs(scaled);
        const int  iv    = abs_v / 10;
        const int  fv    = abs_v % 10;
        std::snprintf(mean_text, sizeof(mean_text), "%c%d.%d", sign, iv, fv);
    }
    std::snprintf(status_text,
                  sizeof(status_text),
                  "%s %s %s M%s",
                  hold_ ? "HLD" : "LIV",
                  use_peak_detect_ ? "PEK" : "SMP",
                  trig_short,
                  mean_text);

    if(has_signal)
    {
        gfx.DrawString(2, 2, top_hint_, trace_color, t.bg);
        gfx.DrawString(82, 2, status_text, t.fg, t.bg);
    }
    else
    {
        gfx.FillRect(0, 0, Gfx::kWidth, LayoutView::kTopBarHeight, trace_color);
        gfx.DrawString(2, 2, top_hint_, t.bg, trace_color);
        gfx.DrawString(82, 2, status_text, t.bg, trace_color);
    }

    out_text = nullptr;
    return false;
}

} // namespace rools
