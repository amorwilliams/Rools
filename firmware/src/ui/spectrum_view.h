#pragma once

#include "display/gfx.h"
#include "dsp/fft_analyzer.h"

namespace rools {

class SpectrumView {
public:
    void Draw(Gfx& gfx, const FftAnalyzer& fft, bool peak_hold, bool fullscreen);

private:
    Color565 BarColor(float level) const;
};

} // namespace rools
