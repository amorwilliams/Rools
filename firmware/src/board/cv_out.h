#pragma once

#include <cstddef>
#include <cstdint>

namespace rools {

struct CvOutputs;

constexpr uint16_t kCvOutDacMaxCode = 65535;
constexpr float    kCvOutVrefVolts  = 2.5f; // DAC8565 内部基准; 满量程 0–2.5V

/** DAC8565 四路 CV 输出; SPI 阻塞写 + LDAC,仅主循环调用。 */
class CvOutDriver {
public:
    void Init();
    void Apply(const CvOutputs& out);

private:
    bool WriteBuffer(size_t ch, uint16_t code16);
    void PulseLdac();

    uint32_t last_[4]{0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF};
    bool     inited_ = false;
};

/** norm -1..1 → 16-bit DAC(0–2.5V 内部基准; OPA4171 调理至 ±10V) */
uint16_t CvOutNormToDacCode(float norm);

} // namespace rools
