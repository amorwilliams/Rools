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
                        bool             fullscreen)
{
    const int top    = fullscreen ? 0 : 12;
    const int bottom = Gfx::kHeight - 2;
    const int height = bottom - top;

    const auto& t = theme::kDefault;
    gfx.FillRect(0, 0, Gfx::kWidth, Gfx::kHeight, t.bg);

    if(!fullscreen)
    {
        gfx.DrawString(2, 2, "Spectrum", t.accent, t.bg);
    }

    const size_t bins = fft.num_bins();
    if(bins == 0)
        return;

    const int bar_w = (Gfx::kWidth - 4) / static_cast<int>(bins);
    const int gap   = 1;

    for(size_t i = 0; i < bins; ++i)
    {
        const float  level = peak_hold ? fft.peaks()[i] : fft.bins()[i];
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

    gfx.DrawRect(0, top, Gfx::kWidth, Gfx::kHeight - top, t.border);
}

} // namespace rools
