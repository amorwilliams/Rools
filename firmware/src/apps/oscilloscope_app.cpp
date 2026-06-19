#include "apps/oscilloscope_app.h"

#include <cmath>
#include <cstdio>

#include "ui/layout_view.h"

namespace rools {

const float OscilloscopeApp::kTimeScalesMsPerDiv[8] = {0.1f, 0.2f, 0.5f, 1.0f, 2.0f, 5.0f, 10.0f, 20.0f};
const float OscilloscopeApp::kVoltScalesPerDiv[6]   = {0.5f, 1.0f, 2.0f, 5.0f, 10.0f, 20.0f};
static size_t s_render_write_idx = 0;

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
    effective_interp_mode_ = interp_mode_;
    draw_cols_      = 0;
    std::snprintf(focus_value_, sizeof(focus_value_), "-");
    std::snprintf(b_hint_, sizeof(b_hint_), "-");
    std::snprintf(top_hint_, sizeof(top_hint_), "Oscilloscope");
    for(size_t i = 0; i < kScopeBufSize; ++i)
    {
        ch1_buffer_[i] = 0.f;
        ch2_buffer_[i] = 0.f;
    }
    for(size_t i = 0; i < kScreenCols; ++i)
    {
        draw_mean_[i] = 0.f;
        draw_min_[i]  = 0.f;
        draw_max_[i]  = 0.f;
    }
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

        if(++decimate_phase_ < kDecimate)
            continue;

        decimate_phase_        = 0;
        ch1_buffer_[write_idx_] = l;
        ch2_buffer_[write_idx_] = r;
        write_idx_             = (write_idx_ + 1) % kScopeBufSize;
        if(sample_count_ < kScopeBufSize)
            ++sample_count_;
    }
}

const float* OscilloscopeApp::ActiveBuffer() const
{
    return input_src_ == InputSrc::Ch1 ? ch1_buffer_ : ch2_buffer_;
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
    if(effective_interp_mode_ == InterpMode::Cubic)
        return InterpolateCubic(src, total, linear_pos);
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
        if(trigger_edge_ == TriggerEdge::Rise)
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
    if(sample_count_ < 2)
    {
        draw_cols_ = 0;
        return;
    }

    s_render_write_idx = write_idx_;

    const float  sample_rate = 48000.f / static_cast<float>(kDecimate);
    const float  ms_per_div  = kTimeScalesMsPerDiv[time_idx_];
    const float  total_ms    = ms_per_div * 12.0f;
    const size_t desired     = static_cast<size_t>((total_ms * sample_rate) / 1000.0f);
    const size_t window      = (desired < 64) ? 64 : (desired > kScopeBufSize ? kScopeBufSize : desired);

    const float* src = ActiveBuffer();
    const size_t total = sample_count_;
    const size_t end_linear = total;
    size_t       start_linear = (end_linear > window) ? (end_linear - window) : 0;
    trigger_subsample_ = 0.f;
    trigger_hit_       = FindTriggerStart(src, end_linear, window, start_linear, trigger_subsample_);
    if(!trigger_hit_)
    {
        if(trigger_mode_ == TriggerMode::Norm)
            return;
        trigger_subsample_ = 0.f;
    }

    draw_cols_ = kScreenCols;
    const float start_pos = static_cast<float>(start_linear) + trigger_subsample_;
    const float step      = static_cast<float>(window) / static_cast<float>(draw_cols_);
    effective_interp_mode_ = interp_mode_;
    if(interp_mode_ == InterpMode::Cubic && window > 2048)
        effective_interp_mode_ = InterpMode::Linear;

    if(render_mode_ == RenderMode::Sample)
    {
        for(size_t x = 0; x < draw_cols_; ++x)
        {
            const float center = start_pos + (static_cast<float>(x) + 0.5f) * step;
            const float v      = InterpolateSample(src, total, center);
            draw_mean_[x]      = v;
            draw_min_[x]       = v;
            draw_max_[x]       = v;
        }
        return;
    }

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
        draw_min_[x]  = min_v;
        draw_max_[x]  = max_v;
        draw_mean_[x] = (cnt == 0) ? 0.f : (sum / static_cast<float>(cnt));
    }
}

void OscilloscopeApp::BuildFocusValueText()
{
    const auto format_one_decimal = [](char* dst, size_t dst_size, float value, const char* suffix) {
        int scaled = static_cast<int>(std::lround(value * 10.0f));
        int ipart  = scaled / 10;
        int fpart  = std::abs(scaled % 10);
        std::snprintf(dst, dst_size, "%d.%d%s", ipart, fpart, suffix);
    };

    switch(focus_param_)
    {
    case FocusParam::InputSrc:
        std::snprintf(focus_value_, sizeof(focus_value_), "%s", input_src_ == InputSrc::Ch1 ? "CH1" : "CH2");
        break;
    case FocusParam::TimeScale:
        format_one_decimal(focus_value_, sizeof(focus_value_), kTimeScalesMsPerDiv[time_idx_], "ms/div");
        break;
    case FocusParam::VoltScale:
        format_one_decimal(focus_value_, sizeof(focus_value_), kVoltScalesPerDiv[volt_idx_], "V/div");
        break;
    case FocusParam::RenderMode:
        std::snprintf(focus_value_, sizeof(focus_value_), "%s", render_mode_ == RenderMode::Sample ? "SAMPLE" : "PEAK");
        break;
    case FocusParam::LineAA:
        std::snprintf(focus_value_, sizeof(focus_value_), "%s", aa_enabled_ ? "AA ON" : "AA OFF");
        break;
    case FocusParam::InterpMode:
        if(interp_mode_ == InterpMode::Cubic && effective_interp_mode_ != InterpMode::Cubic)
            std::snprintf(focus_value_, sizeof(focus_value_), "CUBIC>AUTO LIN");
        else
            std::snprintf(focus_value_, sizeof(focus_value_), "%s", interp_mode_ == InterpMode::Linear ? "LINEAR" : "CUBIC");
        break;
    case FocusParam::TriggerMode:
        std::snprintf(focus_value_, sizeof(focus_value_), "%s", trigger_mode_ == TriggerMode::Auto ? "AUTO" : "NORM");
        break;
    case FocusParam::TriggerLevel:
        std::snprintf(focus_value_, sizeof(focus_value_), "%+.2fV", trigger_level_);
        break;
    case FocusParam::TriggerEdge:
        std::snprintf(focus_value_, sizeof(focus_value_), "%s", trigger_edge_ == TriggerEdge::Rise ? "RISE" : "FALL");
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

    gfx_->SetLineQuality(aa_enabled_ ? Gfx::LineQuality::AntiAliased : Gfx::LineQuality::Fast);
    CaptureWindow();

    char input_label[8];
    char time_label[12];
    char volt_label[12];
    std::snprintf(input_label, sizeof(input_label), "CH%d", input_src_ == InputSrc::Ch1 ? 1 : 2);
    std::snprintf(time_label, sizeof(time_label), "%.1fms", kTimeScalesMsPerDiv[time_idx_]);
    std::snprintf(volt_label, sizeof(volt_label), "%.1fV", kVoltScalesPerDiv[volt_idx_]);
    BuildFocusValueText();

    const OscilloscopeViewState state{
        !hold_,
        hold_,
        fine_mode_,
        input_label,
        time_label,
        volt_label,
        trigger_mode_ == TriggerMode::Auto ? "AUTO" : "NORM",
        trigger_edge_ == TriggerEdge::Rise ? "RISE" : "FALL",
        render_mode_ == RenderMode::Sample ? "SAMPLE" : "PEAK",
        kVoltScalesPerDiv[volt_idx_],
        trigger_level_,
        render_mode_ == RenderMode::PeakDetect,
        layout.main_top,
        layout.main_bottom,
    };
    view_.Draw(*gfx_, state, draw_mean_, draw_min_, draw_max_, draw_cols_, trigger_hit_);
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
            = (trigger_mode_ == TriggerMode::Auto)
              && (candidate == FocusParam::TriggerLevel || candidate == FocusParam::TriggerEdge);
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
    case FocusParam::InputSrc: input_src_ = (step > 0) ? InputSrc::Ch2 : InputSrc::Ch1; break;
    case FocusParam::TimeScale:
        time_idx_ += step;
        if(time_idx_ < 0)
            time_idx_ = 0;
        if(time_idx_ > 7)
            time_idx_ = 7;
        break;
    case FocusParam::VoltScale:
        volt_idx_ += step;
        if(volt_idx_ < 0)
            volt_idx_ = 0;
        if(volt_idx_ > 5)
            volt_idx_ = 5;
        break;
    case FocusParam::RenderMode:
        render_mode_ = (step > 0) ? RenderMode::PeakDetect : RenderMode::Sample;
        break;
    case FocusParam::LineAA:
        aa_enabled_ = step > 0;
        break;
    case FocusParam::InterpMode:
        interp_mode_ = (step > 0) ? InterpMode::Cubic : InterpMode::Linear;
        break;
    case FocusParam::TriggerMode:
        trigger_mode_ = (step > 0) ? TriggerMode::Norm : TriggerMode::Auto;
        if(trigger_mode_ == TriggerMode::Auto
           && (focus_param_ == FocusParam::TriggerLevel || focus_param_ == FocusParam::TriggerEdge))
            focus_param_ = FocusParam::TriggerMode;
        break;
    case FocusParam::TriggerLevel:
        trigger_level_ += static_cast<float>(step) * (fine_mode_ ? 0.05f : 0.25f);
        if(trigger_level_ > 5.f)
            trigger_level_ = 5.f;
        if(trigger_level_ < -5.f)
            trigger_level_ = -5.f;
        break;
    case FocusParam::TriggerEdge: trigger_edge_ = (step > 0) ? TriggerEdge::Fall : TriggerEdge::Rise; break;
    default: break;
    }
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
    case FocusParam::InputSrc: return "Input";
    case FocusParam::TimeScale: return "Time";
    case FocusParam::VoltScale: return "Volt";
    case FocusParam::RenderMode: return "Render";
    case FocusParam::LineAA: return "Line";
    case FocusParam::InterpMode: return "Interp";
    case FocusParam::TriggerMode: return "Trig";
    case FocusParam::TriggerLevel: return "Level";
    case FocusParam::TriggerEdge: return "Edge";
    default: return "Param";
    }
}

const char* OscilloscopeApp::current_b_hint() const
{
    return b_hint_;
}

const char* OscilloscopeApp::current_top_hint() const
{
    std::snprintf(top_hint_,
                  sizeof(top_hint_),
                  "%s %s %s",
                  input_src_ == InputSrc::Ch1 ? "CH1" : "CH2",
                  hold_ ? "HOLD" : "LIVE",
                  render_mode_ == RenderMode::Sample ? "SAMPLE" : "PEAK");
    return top_hint_;
}

} // namespace rools
