#pragma once

#include <cstddef>

#include "display/gfx.h"

namespace rools {

struct OscilloscopeViewState {
    bool        run;
    bool        hold;
    bool        fine_mode;
    const char* input_label;
    const char* time_label;
    const char* volt_label;
    const char* trig_mode_label;
    const char* trig_edge_label;
    const char* render_mode_label;
    float       volt_per_div;
    float       trigger_level_v;
    bool        peak_detect;
    int         main_top;
    int         main_bottom;
};

struct OscilloscopeTraceView {
    const char* label;
    const float* mean_samples;
    const float* min_samples;
    const float* max_samples;
    float       volt_per_div;
    Color565    color;
    bool        has_signal;
    bool        visible;
    bool        selected;
};

class OscilloscopeView {
public:
    void Draw(Gfx&                         gfx,
              const OscilloscopeViewState& state,
              const OscilloscopeTraceView* traces,
              size_t                       trace_count,
              size_t                       sample_count,
              bool                         trigger_active);

private:
    int  SampleToY(float sample_norm, float volt_per_div, int grid_top, int grid_bottom) const;
    void DrawGrid(Gfx& gfx, int top, int bottom) const;
};

} // namespace rools
