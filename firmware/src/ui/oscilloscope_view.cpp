#include "ui/oscilloscope_view.h"

#include <cmath>
#include <cstdio>

#include "display/theme.h"

namespace rools {

static Color565 ScaleColor(Color565 c, uint8_t num, uint8_t den)
{
    const uint16_t v = c.v;
    uint8_t        r = static_cast<uint8_t>((v >> 11) & 0x1F);
    uint8_t        g = static_cast<uint8_t>((v >> 5) & 0x3F);
    uint8_t        b = static_cast<uint8_t>(v & 0x1F);
    r = static_cast<uint8_t>((static_cast<uint16_t>(r) * num) / den);
    g = static_cast<uint8_t>((static_cast<uint16_t>(g) * num) / den);
    b = static_cast<uint8_t>((static_cast<uint16_t>(b) * num) / den);
    return Color565(static_cast<uint16_t>((r << 11) | (g << 5) | b));
}

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
        gfx.DrawVLine(x, top, height, (c == cols / 2) ? t.fg : color::Rgb(40, 48, 68));
    }
    for(int r = 0; r <= rows; ++r)
    {
        const int y = top + (r * height) / rows;
        gfx.DrawHLine(0, y, width, (r == rows / 2 || r == rows) ? t.fg : color::Rgb(40, 48, 68));
    }
}

int OscilloscopeView::SampleToY(float sample_norm,
                                float volt_per_div,
                                float full_scale_volts,
                                int   grid_top,
                                int   grid_bottom) const
{
    const int grid_h      = grid_bottom - grid_top;
    const int mid_y       = grid_top + grid_h / 2;
    const float v         = sample_norm * full_scale_volts;
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
                            const OscilloscopeTraceView* traces,
                            size_t                       trace_count,
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

    if(state.trig_mode_label[0] != 'A')
    {
        const int trig_y = SampleToY(state.trigger_level_v / state.trigger_full_scale_volts,
                                     state.volt_per_div,
                                     state.trigger_full_scale_volts,
                                     grid_top,
                                     grid_bottom);
        gfx.DrawHLine(0, trig_y, Gfx::kWidth, trigger_active ? t.peak : t.border);
    }

    if(sample_count > 1)
    {
        const int width = static_cast<int>(sample_count);
        for(size_t ti = 0; ti < trace_count; ++ti)
        {
            const OscilloscopeTraceView& trace = traces[ti];
            if(!trace.visible || !trace.has_signal)
                continue;
            const Color565 wave_color = trace.selected ? trace.color : ScaleColor(trace.color, 1, 2);
            for(int x = 0; x < width; ++x)
            {
                const int y_mean = SampleToY(trace.mean_samples[x],
                                             trace.volt_per_div,
                                             trace.full_scale_volts,
                                             grid_top,
                                             grid_bottom);
                if(state.peak_detect)
                {
                    const int y0 = SampleToY(trace.min_samples[x],
                                            trace.volt_per_div,
                                            trace.full_scale_volts,
                                            grid_top,
                                            grid_bottom);
                    const int y1 = SampleToY(trace.max_samples[x],
                                            trace.volt_per_div,
                                            trace.full_scale_volts,
                                            grid_top,
                                            grid_bottom);
                    const int y_min = (y0 < y1) ? y0 : y1;
                    const int y_max = (y0 > y1) ? y0 : y1;
                    DrawThickV(x, y_min, y_max, wave_color);
                    DrawThickV(x, y_mean, y_mean, wave_color);
                }
                else
                {
                    const int nx  = (x + 1 < width) ? (x + 1) : x;
                    const int y1n = SampleToY(trace.mean_samples[nx],
                                              trace.volt_per_div,
                                              trace.full_scale_volts,
                                              grid_top,
                                              grid_bottom);
                    DrawThickLine(x, y_mean, nx, y1n, wave_color);
                }
            }
        }
    }

}

} // namespace rools
