#pragma once

#include "board/pins.h"
#include "per/gpio.h"
#include "per/spi.h"

namespace rools {

class St7735 {
public:
    static constexpr int kWidth  = 160;
    static constexpr int kHeight = 128;

    St7735() = default;

    void Init();
    void SetBacklight(bool on);

    void SetAddrWindow(int x, int y, int w, int h);
    void WritePixels(const uint16_t* data, size_t count);
    void FillScreen(uint16_t color);

    uint16_t* framebuffer() { return framebuffer_; }

private:
    void Reset();
    void WriteCommand(uint8_t cmd);
    void WriteData(uint8_t data);
    void WriteData16(uint16_t data);
    void InitSequence();
    void ApplyRotation();

    daisy::SpiHandle spi_;
    daisy::GPIO      cs_;
    daisy::GPIO      dc_;
    daisy::GPIO      rst_;
    daisy::GPIO      blk_;

    uint16_t framebuffer_[kWidth * kHeight];

    static constexpr int kColOffset = 2;
    static constexpr int kRowOffset = 1;
};

} // namespace rools
