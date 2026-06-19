#include "ui/spectrum_view.h"

#include "display/theme.h"

namespace rools {

Color565 SpectrumView::BarColor(float level) const
{
    if(level < 0.33f)
        return theme::kDefault.bar_low;
    if(level < 0.66f)
        return theme::kDefault.bar_mid;
    return theme::kDefault.bar_high;
}

void SpectrumView::Draw(Gfx&           gfx,
                        const FftAnalyzer& fft,
                        bool             peak_hold,
                        bool             fullscreen,
                        int              main_top,
                        int              main_bottom)
{
    const int top    = fullscreen ? main_top : main_top;
    const int bottom = main_bottom - 1;
    const int height = bottom - top;

    const auto& t = theme::kDefault;
    gfx.FillRect(0, main_top, Gfx::kWidth, main_bottom - main_top, t.bg);

    if(!fullscreen)
    {
        gfx.DrawString(2, top + 2, "Spectrum", t.accent, t.bg);
    }

    // Always draw a baseline/grid so zero input is visually obvious.
    const int baseline_y = bottom;
    const int mid_y      = top + height / 2;
    gfx.DrawHLine(0, baseline_y, Gfx::kWidth, t.border);
    gfx.DrawHLine(0, mid_y, Gfx::kWidth, t.muted);

    const size_t bins = fft.num_bins();
    if(bins == 0)
        return;

    const int bar_w = (Gfx::kWidth - 4) / static_cast<int>(bins);
    const int gap   = 1;

    float max_level = 0.f;
    for(size_t i = 0; i < bins; ++i)
    {
        const float  level = peak_hold ? fft.peaks()[i] : fft.bins()[i];
        if(level > max_level)
            max_level = level;
        const int    h     = static_cast<int>(level * static_cast<float>(height));
        const int    x     = 2 + static_cast<int>(i) * bar_w;
        const int    y     = bottom - h;
        const Color565 c   = BarColor(level);

        gfx.FillRect(x, y, bar_w - gap, h, c);

        if(peak_hold && fft.peaks()[i] > fft.bins()[i])
        {
            const int py = bottom - static_cast<int>(fft.peaks()[i] * height);
            gfx.DrawHLine(x, py, bar_w - gap, t.peak);
        }
    }

    if(max_level < 0.01f)
        gfx.DrawString(2, top + 2, "NO SIGNAL", t.muted, t.bg);

    gfx.DrawRect(0, top, Gfx::kWidth, main_bottom - top, t.border);
}

} // namespace rools
