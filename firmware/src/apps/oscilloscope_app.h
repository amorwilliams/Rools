#pragma once

#include <cstddef>
#include <cstdint>

#include "app_shell.h"
#include "display/gfx.h"
#include "ui/oscilloscope_view.h"

namespace rools {

class OscilloscopeApp : public App {
public:
    void Bind(Gfx* gfx);

    const char* name() const override { return "Oscilloscope"; }

    void on_enter() override;
    void on_exit() override;

    void audio_callback(const float* inL,
                        const float* inR,
                        float*       outL,
                        float*       outR,
                        size_t       n) override;

    void ui_draw(const LayoutMetrics& layout) override;
    void on_enc(Enc enc, int delta) override;
    void on_enc_press(Enc enc, bool pressed) override;
    void on_enc_shift(Enc enc, int delta) override;
    void on_btn(Btn btn, bool pressed) override;

    const ParamMap* param_map() const override;
    const char*     current_a_hint() const override;
    const char*     current_b_hint() const override;
    const char*     current_button_hint() const override { return hold_ ? "Unhold" : "Hold"; }
    const char*     current_button_shift_hint() const override { return hold_ ? "Unhold" : "Hold"; }
    const char*     current_shift_hint() const override { return "Hold"; }
    const char*     current_top_hint() const override;
    uint32_t        ui_refresh_interval_ms() const override
    {
        if(hold_)
            return 100;
        if(interp_mode_ == InterpMode::Cubic && aa_enabled_)
            return 40;
        return 33;
    }

private:
    enum class FocusParam : uint8_t {
        InputSrc,
        TimeScale,
        VoltScale,
        RenderMode,
        LineAA,
        InterpMode,
        TriggerMode,
        TriggerLevel,
        TriggerEdge,
        Count
    };
    enum class InputSrc : uint8_t { Ch1, Ch2 };
    enum class TriggerMode : uint8_t { Auto, Norm };
    enum class TriggerEdge : uint8_t { Rise, Fall };
    enum class RenderMode : uint8_t { Sample, PeakDetect };
    enum class InterpMode : uint8_t { Linear, Cubic };

    static constexpr size_t kDecimate      = 2;
    static constexpr size_t kScopeBufSize  = 4096;
    static constexpr size_t kScreenCols    = 160;

    static const float kTimeScalesMsPerDiv[8];
    static const float kVoltScalesPerDiv[6];

    Gfx*             gfx_ = nullptr;
    OscilloscopeView view_;

    float ch1_buffer_[kScopeBufSize];
    float ch2_buffer_[kScopeBufSize];
    size_t write_idx_      = 0;
    size_t sample_count_   = 0;
    size_t decimate_phase_ = 0;

    FocusParam   focus_param_   = FocusParam::InputSrc;
    InputSrc     input_src_     = InputSrc::Ch1;
    TriggerMode  trigger_mode_  = TriggerMode::Auto;
    TriggerEdge  trigger_edge_  = TriggerEdge::Rise;
    RenderMode   render_mode_   = RenderMode::Sample;
    InterpMode   interp_mode_   = InterpMode::Cubic;
    InterpMode   effective_interp_mode_ = InterpMode::Cubic;
    bool         aa_enabled_    = true;
    bool         hold_          = false;
    bool         fine_mode_     = false;
    int          time_idx_      = 3;
    int          volt_idx_      = 1;
    float        trigger_level_ = 0.0f; // volts
    bool         trigger_hit_   = false;
    float        trigger_subsample_ = 0.f;

    float draw_mean_[kScreenCols];
    float draw_min_[kScreenCols];
    float draw_max_[kScreenCols];
    size_t draw_cols_ = 0;
    char   focus_value_[24];
    char   b_hint_[32];
    mutable char top_hint_[48];

    void AdvanceFocus(int delta);
    void AdjustFocusedParam(int delta);
    void CaptureWindow();
    void BuildFocusValueText();
    bool FindTriggerStart(const float* src, size_t total, size_t window, size_t& start, float& subsample) const;
    const float* ActiveBuffer() const;
    float SampleLinearIndex(const float* src, size_t total, size_t linear_idx) const;
    float InterpolateLinear(const float* src, size_t total, float linear_pos) const;
    float InterpolateCubic(const float* src, size_t total, float linear_pos) const;
    float InterpolateSample(const float* src, size_t total, float linear_pos) const;
};

} // namespace rools
