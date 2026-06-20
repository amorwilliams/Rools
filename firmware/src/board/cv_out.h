#pragma once

#include <cstddef>
#include <cstdint>

#include "per/i2c.h"

namespace rools {

struct CvOutputs;

constexpr uint8_t  kMcp4728I2cAddr   = 0x60; // 7-bit
constexpr uint16_t kCvOutDacMaxCode  = 4095;
constexpr float    kCvOutVrefVolts   = 3.3f; // VREF=VDD (ADR-006 情况 A)

/** MCP4728 四路 CV 输出;I2C 阻塞写,仅主循环调用。 */
class CvOutDriver {
public:
    void Init();
    void Apply(const CvOutputs& out);

private:
    bool WriteChannel(size_t ch, uint16_t code12);

    daisy::I2CHandle i2c_;
    uint16_t         last_[4]{0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF};
    bool             inited_ = false;
};

/** norm -1..1 → 12-bit DAC(VDD ref, 0..3.3V 单极性,前端再调理至 ±10V) */
uint16_t CvOutNormToDacCode(float norm);

} // namespace rools
