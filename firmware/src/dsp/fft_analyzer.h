#pragma once

#include "board/audio_config.h"

#include <cstddef>

namespace rools {

class FftAnalyzer {
public:
    static constexpr size_t kMaxFftSize = 1024;
    static constexpr size_t kMaxOutBins = 64;

    FftAnalyzer();

    void SetFftSize(size_t n);
    void SetGainDb(float db);
    void SetDecay(float decay); // 0..1 per frame

    /** Feed time-domain samples; runs FFT when enough data available. */
    void ProcessBlock(const float* samples, size_t count);

    size_t fft_size() const { return fft_size_; }
    size_t num_bins() const { return num_bins_; }

    const float* bins() const { return bins_; }
    const float* peaks() const { return peaks_; }

private:
    void RunFft();
    void BuildWindow();
    void MapToDisplayBins();

    size_t fft_size_;
    size_t num_bins_;
    float  gain_db_;
    float  decay_;

    float window_[kMaxFftSize];
    float work_[kMaxFftSize];
    float real_[kMaxFftSize];
    float imag_[kMaxFftSize];
    float mag_[kMaxFftSize / 2 + 1];
    float bins_[kMaxOutBins];
    float peaks_[kMaxOutBins];

    size_t fill_;
};

} // namespace rools
