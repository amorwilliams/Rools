#pragma once

#include <atomic>
#include <cstddef>
#include <cstring>

namespace rools {

/** Single-producer (audio ISR) / single-consumer (main loop) float ring. */
template <size_t Capacity>
class SampleRing {
public:
    SampleRing() { Reset(); }

    void Reset()
    {
        write_idx_.store(0, std::memory_order_relaxed);
        read_idx_.store(0, std::memory_order_relaxed);
    }

    void Push(float sample)
    {
        const size_t w = write_idx_.load(std::memory_order_relaxed);
        buffer_[w % Capacity] = sample;
        write_idx_.store(w + 1, std::memory_order_release);
    }

    /** Copy up to max_samples from ring; returns count copied. */
    size_t Pop(float* dst, size_t max_samples)
    {
        const size_t w = write_idx_.load(std::memory_order_acquire);
        size_t       r = read_idx_.load(std::memory_order_relaxed);
        size_t       n = 0;

        while(r < w && n < max_samples)
        {
            dst[n++] = buffer_[r % Capacity];
            ++r;
        }

        read_idx_.store(r, std::memory_order_release);
        return n;
    }

    size_t Available() const
    {
        const size_t w = write_idx_.load(std::memory_order_acquire);
        const size_t r = read_idx_.load(std::memory_order_acquire);
        return (w > r) ? (w - r) : 0;
    }

private:
    float                buffer_[Capacity];
    std::atomic<size_t>  write_idx_;
    std::atomic<size_t>  read_idx_;
};

} // namespace rools
