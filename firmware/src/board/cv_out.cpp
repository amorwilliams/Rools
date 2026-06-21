#include "board/cv_out.h"

#include "app_shell.h"
#include "board/cv_reference.h"
#include "board/pins.h"
#include "board/spi1_bus.h"
#include "per/gpio.h"
#include "per/spi.h"

namespace rools {

using daisy::GPIO;
using daisy::SpiHandle;

namespace {

GPIO g_dac_cs;
GPIO g_dac_ldac;

bool SpiWrite24(uint32_t frame24)
{
    if(!Spi1Bus::IsReady())
        return false;

    uint8_t buf[3] = {
        static_cast<uint8_t>((frame24 >> 16) & 0xFF),
        static_cast<uint8_t>((frame24 >> 8) & 0xFF),
        static_cast<uint8_t>(frame24 & 0xFF),
    };

    g_dac_cs.Write(false);
    const auto result = Spi1Bus::Handle().BlockingTransmit(buf, 3, 10);
    g_dac_cs.Write(true);
    return result == SpiHandle::Result::OK;
}

} // namespace

uint16_t CvOutNormToDacCode(float norm)
{
    norm              = ClampBipolar(norm);
    const float dac_uni = (1.f - norm) * 0.5f; // 0V→+10V, 2.5V→−10V (OPA4171)
    const int   code    = static_cast<int>(dac_uni * static_cast<float>(kCvOutDacMaxCode) + 0.5f);
    if(code < 0)
        return 0;
    if(code > static_cast<int>(kCvOutDacMaxCode))
        return kCvOutDacMaxCode;
    return static_cast<uint16_t>(code);
}

void CvOutDriver::Init()
{
    GPIO::Config gcfg;
    gcfg.mode = GPIO::Mode::OUTPUT;
    gcfg.pull = GPIO::Pull::NOPULL;

    gcfg.pin = pins::kDacCs;
    g_dac_cs.Init(gcfg);
    g_dac_cs.Write(true);

    gcfg.pin = pins::kDacLdac;
    g_dac_ldac.Init(gcfg);
    g_dac_ldac.Write(true); // LDAC 低有效; 高=锁定, SPI 写不更新输出

    inited_ = Spi1Bus::IsReady();
}

bool CvOutDriver::WriteBuffer(size_t ch, uint16_t code16)
{
    if(!inited_ || ch > 3)
        return false;

    // DB23:16 = 0; DB21:20 = 00 store; DB19 = 0; DB18:17 = ch; DB16 = 0
    const uint32_t frame = (static_cast<uint32_t>(ch & 3) << 17) | static_cast<uint32_t>(code16);
    return SpiWrite24(frame);
}

void CvOutDriver::PulseLdac()
{
    g_dac_ldac.Write(false);
    g_dac_ldac.Write(true);
}

void CvOutDriver::Apply(const CvOutputs& out)
{
    if(!inited_)
        return;

    const float    norms[4]  = {out.a, out.b, out.c, out.d};
    const uint16_t codes[4] = {CvOutNormToDacCode(norms[0]),
                               CvOutNormToDacCode(norms[1]),
                               CvOutNormToDacCode(norms[2]),
                               CvOutNormToDacCode(norms[3])};

    bool     changed = false;
    bool     ok      = true;
    for(size_t i = 0; i < 4; ++i)
    {
        if(static_cast<uint32_t>(codes[i]) == last_[i])
            continue;
        if(!WriteBuffer(i, codes[i]))
            ok = false;
        else
        {
            last_[i] = codes[i];
            changed  = true;
        }
    }

    if(changed && ok)
        PulseLdac();
}

} // namespace rools
