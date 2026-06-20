#include "board/cv_out.h"
#include "app_shell.h"
#include "board/cv_reference.h"
#include "board/pins.h"
#include "per/i2c.h"

namespace rools {

using daisy::I2CHandle;

uint16_t CvOutNormToDacCode(float norm)
{
    norm           = ClampBipolar(norm);
    const float uni = (norm + 1.f) * 0.5f;
    const int   code = static_cast<int>(uni * static_cast<float>(kCvOutDacMaxCode) + 0.5f);
    if(code < 0)
        return 0;
    if(code > static_cast<int>(kCvOutDacMaxCode))
        return kCvOutDacMaxCode;
    return static_cast<uint16_t>(code);
}

void CvOutDriver::Init()
{
    I2CHandle::Config cfg{};
    cfg.periph             = I2CHandle::Config::Peripheral::I2C_1;
    cfg.mode               = I2CHandle::Config::Mode::I2C_MASTER;
    cfg.speed              = I2CHandle::Config::Speed::I2C_400KHZ;
    cfg.pin_config.scl     = pins::kI2cScl;
    cfg.pin_config.sda     = pins::kI2cSda;

    inited_ = (i2c_.Init(cfg) == I2CHandle::Result::OK);
}

bool CvOutDriver::WriteChannel(size_t ch, uint16_t code12)
{
    if(!inited_ || ch > 3)
        return false;

    code12 &= kCvOutDacMaxCode;

    // MCP4728 Write DAC Input Register: VREF=VDD, PD=normal, gain=1x
    uint8_t data[3];
    data[0] = static_cast<uint8_t>(0x40 | ((ch & 0x03) << 1));
    data[1] = static_cast<uint8_t>((code12 >> 8) & 0x0F);
    data[2] = static_cast<uint8_t>(code12 & 0xFF);

    return i2c_.TransmitBlocking(kMcp4728I2cAddr, data, 3, 10) == I2CHandle::Result::OK;
}

void CvOutDriver::Apply(const CvOutputs& out)
{
    if(!inited_)
        return;

    const float   norms[4] = {out.a, out.b, out.c, out.d};
    const uint16_t codes[4] = {CvOutNormToDacCode(norms[0]),
                               CvOutNormToDacCode(norms[1]),
                               CvOutNormToDacCode(norms[2]),
                               CvOutNormToDacCode(norms[3])};

    for(size_t i = 0; i < 4; ++i)
    {
        if(codes[i] == last_[i])
            continue;
        if(WriteChannel(i, codes[i]))
            last_[i] = codes[i];
    }
}

} // namespace rools
