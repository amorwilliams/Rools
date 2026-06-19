#include "ui/oscilloscope_view.h"

#include <cmath>
#include <cstdio>

#include "display/theme.h"

namespace rools {

void OscilloscopeView::DrawGrid(Gfx& gfx, int top, int bottom) const
{
    const auto& t      = theme::kDefault;
    const int   width  = Gfx::kWidth;
    const int   height = bottom - top;
    const int   cols   = 12;
    const int   rows   = 8;

    for(int c = 0; c <= cols; ++c)
    {
        const int x = (c * (width - 1)) / cols;
        gfx.DrawVLine(x, top, height, (c == cols / 2) ?  t.muted : t.border);
    }
    for(int r = 0; r <= rows; ++r)
    {
        const int y = top + (r * height) / rows;
        gfx.DrawHLine(0, y, width, (r == rows / 2 || r == rows) ? t.muted : t.border);
    }
}

int OscilloscopeView::SampleToY(float sample_norm, float volt_per_div, int grid_top, int grid_bottom) const
{
    const int grid_h      = grid_bottom - grid_top;
    const int mid_y       = grid_top + grid_h / 2;
    const float v         = sample_norm * 5.0f; // Seed codec full-scale as +/-5V reference.
    const float divs      = v / volt_per_div;
    const float px_per_div = static_cast<float>(grid_h) / 8.0f;
    int         y         = static_cast<int>(std::lround(static_cast<float>(mid_y) - divs * px_per_div));
    if(y < grid_top)
        y = grid_top;
    if(y > grid_bottom)
        y = grid_bottom;
    return y;
}

void OscilloscopeView::Draw(Gfx&                         gfx,
                            const OscilloscopeViewState& state,
                            const float*                 mean_samples,
                            const float*                 min_samples,
                            const float*                 max_samples,
                            size_t                       sample_count,
                            bool                         trigger_active)
{
    const auto& t           = theme::kDefault;
    const int   grid_top    = state.main_top;
    const int   grid_bottom = state.main_bottom - 1;
    auto DrawThickV = [&](int x, int y0, int y1, Color565 c) {
        if(y1 < y0)
        {
            const int tmp = y0;
            y0            = y1;
            y1            = tmp;
        }
        gfx.DrawVLine(x, y0, y1 - y0 + 1, c);
        if(x + 1 < Gfx::kWidth)
            gfx.DrawVLine(x + 1, y0, y1 - y0 + 1, c);
    };
    auto DrawThickLine = [&](int x0, int y0, int x1, int y1, Color565 c) {
        gfx.DrawLine(x0, y0, x1, y1, c);
        if(y0 + 1 <= grid_bottom && y1 + 1 <= grid_bottom)
            gfx.DrawLine(x0, y0 + 1, x1, y1 + 1, c);
    };

    gfx.FillRect(0, grid_top, Gfx::kWidth, grid_bottom - grid_top + 1, t.bg);

    DrawGrid(gfx, grid_top, grid_bottom);

    if(state.trig_mode_label[0] == 'N')
    {
        const int trig_y = SampleToY(state.trigger_level_v / 5.0f, state.volt_per_div, grid_top, grid_bottom);
        gfx.DrawHLine(0, trig_y, Gfx::kWidth, trigger_active ? t.peak : t.border);
    }

    if(sample_count > 1)
    {
        const int width = static_cast<int>(sample_count);
        for(int x = 0; x < width; ++x)
        {
            const int y_mean = SampleToY(mean_samples[x], state.volt_per_div, grid_top, grid_bottom);
            if(state.peak_detect)
            {
                const int y0 = SampleToY(min_samples[x], state.volt_per_div, grid_top, grid_bottom);
                const int y1 = SampleToY(max_samples[x], state.volt_per_div, grid_top, grid_bottom);
                const int y_min = (y0 < y1) ? y0 : y1;
                const int y_max = (y0 > y1) ? y0 : y1;
                DrawThickV(x, y_min, y_max, t.bar_high);
                DrawThickV(x, y_mean, y_mean, t.peak);
            }
            else
            {
                const int nx = (x + 1 < width) ? (x + 1) : x;
                const int y1 = SampleToY(mean_samples[nx], state.volt_per_div, grid_top, grid_bottom);
                DrawThickLine(x, y_mean, nx, y1, t.bar_high);
            }
        }
    }

    if(sample_count <= 1)
        gfx.DrawString(2, grid_top + 2, "NO SIGNAL", t.muted, t.bg);

}

} // namespace rools
