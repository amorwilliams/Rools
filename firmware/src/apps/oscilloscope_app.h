#pragma once

#include <cstddef>
#include <cstdint>

#include "app_shell.h"
#include "display/gfx.h"
#include "ui/oscilloscope_view.h"

namespace rools {

class OscilloscopeApp : public App {
public:
    enum class InputSource : uint8_t { Audio1, Audio2, Cv1, Cv2, Cv3, Cv4, None };
    enum class PriorityMode : uint8_t { AudioFirst, CvFirst };
    static constexpr size_t kInputSourceCount = 6;
    static constexpr size_t kMaxDisplayTraces = 4;

    void Bind(Gfx* gfx);

    const char* name() const override { return "Scope"; }

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
    void on_enc_press_shift(Enc enc, bool pressed) override;
    void on_enc_shift(Enc enc, int delta) override;
    void on_btn(Btn btn, bool pressed) override;

    const ParamMap* param_map() const override;
    const char*     current_a_hint() const override;
    const char*     current_b_hint() const override;
    const char*     current_button_hint() const override { return hold_ ? "Unhold" : "Hold"; }
    const char*     current_button_shift_hint() const override { return hold_ ? "Unhold" : "Hold"; }
    const char*     current_shift_hint() const override { return "Hold"; }
    bool            current_top_hint(Gfx& gfx, const char*& out_text) const override;
    uint32_t        ui_refresh_interval_ms() const override
    {
        if(hold_)
            return 100;
        return 16;
    }

private:
    enum class FocusParam : uint8_t {
        TimeScale,
        VoltScale,
        TriggerSource,
        TriggerMode,
        TriggerLevel,
        Priority,
        Count
    };
    enum class TriggerMode : uint8_t { Auto, Rise, Fall };
    struct TraceConfig {
        int volt_idx;
    };

    static constexpr size_t kScopeBufSize  = 4096;
    static constexpr size_t kScreenCols    = 160;
    static constexpr size_t kTimeScaleCount = 14;
    static constexpr size_t kVoltScaleCount = 10;

    static const float  kTimeScalesMsPerDiv[kTimeScaleCount];
    static const size_t kTimeDecimates[kTimeScaleCount];
    static const float kVoltScalesPerDiv[kVoltScaleCount];

    Gfx*             gfx_ = nullptr;
    OscilloscopeView view_;

    float input_buffers_[kInputSourceCount][kScopeBufSize];
    size_t write_idx_      = 0;
    size_t sample_count_   = 0;
    size_t decimate_phase_ = 0;

    FocusParam   focus_param_   = FocusParam::TimeScale;
    PriorityMode priority_mode_ = PriorityMode::AudioFirst;
    TriggerMode  trigger_mode_  = TriggerMode::Auto;
    bool         aa_enabled_    = true;
    bool         hold_          = false;
    bool         fine_mode_     = false;
    int          time_idx_      = 3;
    float        trigger_level_ = 0.0f; // volts
    bool         trigger_hit_   = false;
    float        trigger_subsample_ = 0.f;
    bool         use_peak_detect_ = false;
    int          trigger_stability_score_ = 0;
    bool         smart_audio_mode_ = true;
    InputSource  display_traces_[kMaxDisplayTraces];
    bool         display_trace_has_signal_[kMaxDisplayTraces];
    TraceConfig  trace_cfg_[kMaxDisplayTraces];
    size_t       selected_trace_idx_ = 0;
    size_t       trigger_source_slot_ = 0;

    float  draw_mean_[kMaxDisplayTraces][kScreenCols];
    float  draw_min_[kMaxDisplayTraces][kScreenCols];
    float  draw_max_[kMaxDisplayTraces][kScreenCols];
    float  trace_dc_mean_[kMaxDisplayTraces];
    size_t draw_cols_ = 0;
    char   focus_value_[24];
    char   b_hint_[32];
    mutable char top_hint_[48];

    void AdvanceFocus(int delta);
    void AdjustFocusedParam(int delta);
    void SelectTrace(int dir);
    void CaptureWindow();
    void UpdateDisplayRouting();
    void BuildFocusValueText();
    bool ShouldAutoPeakMode() const;
    bool FindTriggerStart(const float* src,
                          size_t       total,
                          size_t       window,
                          size_t&      start,
                          float&       subsample,
                          float        full_scale_volts) const;
    const float* SourceBuffer(InputSource src) const;
    bool IsDisplaySlotVisible(size_t slot) const;
    bool DetectAudioLike(const float* src, size_t total, size_t window);
    float SampleLinearIndex(const float* src, size_t total, size_t linear_idx) const;
    float InterpolateLinear(const float* src, size_t total, float linear_pos) const;
    float InterpolateCubic(const float* src, size_t total, float linear_pos) const;
    float InterpolateSample(const float* src, size_t total, float linear_pos) const;
};

} // namespace rools
