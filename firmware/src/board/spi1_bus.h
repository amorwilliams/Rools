#pragma once

#include "per/spi.h"

namespace rools {

/** SPI1 单例 — ST7735 与 DAC8565 共用 SCK/MOSI（独立 CS）。 */
class Spi1Bus {
public:
    static daisy::SpiHandle::Result InitDefault();
    static daisy::SpiHandle&       Handle();
    static bool                    IsReady();

private:
    static daisy::SpiHandle handle_;
    static bool             inited_;
};

} // namespace rools
