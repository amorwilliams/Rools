#include "dsp/fft_analyzer.h"

#include <cmath>
#include <cstring>

namespace rools {

namespace {

constexpr float kPi = 3.14159265358979323846f;

void BitReverse(float* real, float* imag, size_t n)
{
    size_t j = 0;
    for(size_t i = 0; i < n; ++i)
    {
        if(i < j)
        {
            const float tr = real[i];
            const float ti = imag[i];
            real[i] = real[j];
            imag[i] = imag[j];
            real[j] = tr;
            imag[j] = ti;
        }
        size_t m = n >> 1;
        while(m >= 1 && j >= m)
        {
            j -= m;
            m >>= 1;
        }
        j += m;
    }
}

void FftInPlace(float* real, float* imag, size_t n)
{
    BitReverse(real, imag, n);

    for(size_t len = 2; len <= n; len <<= 1)
    {
        const float angle = -2.f * kPi / static_cast<float>(len);
        const float wlen_r = cosf(angle);
        const float wlen_i = sinf(angle);

        for(size_t i = 0; i < n; i += len)
        {
            float wr = 1.f;
            float wi = 0.f;
            for(size_t j = 0; j < len / 2; ++j)
            {
                const size_t u = i + j;
                const size_t v = i + j + len / 2;

                const float tr = wr * real[v] - wi * imag[v];
                const float ti = wr * imag[v] + wi * real[v];

                real[v] = real[u] - tr;
                imag[v] = imag[u] - ti;
                real[u] += tr;
                imag[u] += ti;

                const float nwr = wr * wlen_r - wi * wlen_i;
                wi              = wr * wlen_i + wi * wlen_r;
                wr              = nwr;
            }
        }
    }
}

float ToDb(float mag)
{
    const float floor = 1e-9f;
    return 20.f * log10f(mag + floor);
}

} // namespace

FftAnalyzer::FftAnalyzer()
{
    SetFftSize(512);
    SetGainDb(0.f);
    SetDecay(0.65f);
    fill_ = 0;
}

void FftAnalyzer::SetFftSize(size_t n)
{
    if(n != 512 && n != 1024)
        n = 512;

    fft_size_ = n;
    num_bins_ = (n >= 1024) ? 64 : 32;
    fill_     = 0;
    BuildWindow();
    std::memset(bins_, 0, sizeof(bins_));
    std::memset(peaks_, 0, sizeof(peaks_));
}

void FftAnalyzer::SetGainDb(float db)
{
    gain_db_ = db;
}

void FftAnalyzer::SetDecay(float decay)
{
    decay_ = decay;
    if(decay_ < 0.f)
        decay_ = 0.f;
    if(decay_ > 0.99f)
        decay_ = 0.99f;
}

void FftAnalyzer::BuildWindow()
{
    for(size_t i = 0; i < fft_size_; ++i)
    {
        window_[i]
            = 0.5f * (1.f - cosf(2.f * kPi * static_cast<float>(i)
                                 / static_cast<float>(fft_size_ - 1)));
    }
}

void FftAnalyzer::ProcessBlock(const float* samples, size_t count)
{
    for(size_t i = 0; i < count; ++i)
    {
        work_[fill_++] = samples[i];
        if(fill_ >= fft_size_)
        {
            RunFft();
            fill_ = 0;
        }
    }
}

void FftAnalyzer::RunFft()
{
    for(size_t i = 0; i < fft_size_; ++i)
    {
        real_[i] = work_[i] * window_[i];
        imag_[i] = 0.f;
    }

    FftInPlace(real_, imag_, fft_size_);

    const size_t half = fft_size_ / 2;
    for(size_t i = 0; i <= half; ++i)
    {
        const float re = real_[i];
        const float im = imag_[i];
        mag_[i]        = sqrtf(re * re + im * im) / static_cast<float>(fft_size_);
    }

    MapToDisplayBins();
}

void FftAnalyzer::MapToDisplayBins()
{
    const size_t half = fft_size_ / 2;

    for(size_t b = 0; b < num_bins_; ++b)
    {
        const size_t i0 = (b * half) / num_bins_;
        const size_t i1 = ((b + 1) * half) / num_bins_;
        float        peak = 0.f;
        for(size_t i = i0; i < i1 && i <= half; ++i)
        {
            if(mag_[i] > peak)
                peak = mag_[i];
        }

        float db = ToDb(peak) + gain_db_;
        if(db < 0.f)
            db = 0.f;
        if(db > 60.f)
            db = 60.f;

        bins_[b] = db / 60.f;

        peaks_[b] = peaks_[b] * decay_;
        if(bins_[b] > peaks_[b])
            peaks_[b] = bins_[b];
    }
}

} // namespace rools
