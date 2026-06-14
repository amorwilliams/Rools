#include "display/st7735.h"

#include "sys/system.h"

#include <cstring>

namespace rools {

namespace {

enum Cmd : uint8_t {
    SWRESET = 0x01,
    SLPOUT  = 0x11,
    NORON   = 0x13,
    INVOFF  = 0x20,
    DISPON  = 0x29,
    CASET   = 0x2A,
    RASET   = 0x2B,
    RAMWR   = 0x2C,
    MADCTL  = 0x36,
    COLMOD  = 0x3A,
};

} // namespace

void St7735::Init()
{
    daisy::GPIO::Config gcfg;
    gcfg.mode = daisy::GPIO::Mode::OUTPUT;
    gcfg.pull = daisy::GPIO::Pull::NOPULL;

    gcfg.pin = pins::kLcdCs;
    cs_.Init(gcfg);
    cs_.Write(true);

    gcfg.pin = pins::kLcdDc;
    dc_.Init(gcfg);

    gcfg.pin = pins::kLcdRst;
    rst_.Init(gcfg);

    gcfg.pin = pins::kLcdBlk;
    blk_.Init(gcfg);

    daisy::SpiHandle::Config scfg;
    scfg.periph              = daisy::SpiHandle::Config::Peripheral::SPI_1;
    scfg.mode                = daisy::SpiHandle::Config::Mode::MASTER;
    scfg.direction           = daisy::SpiHandle::Config::Direction::TWO_LINES_TX_ONLY;
    scfg.nss                 = daisy::SpiHandle::Config::NSS::SOFT;
    scfg.baud_prescaler      = daisy::SpiHandle::Config::BaudPrescaler::PS_8;
    scfg.pin_config.sclk     = pins::kLcdSck;
    scfg.pin_config.mosi     = pins::kLcdMosi;
    scfg.pin_config.miso     = daisy::Pin();
    scfg.pin_config.nss      = daisy::Pin();
    spi_.Init(scfg);

    Reset();
    InitSequence();
    SetBacklight(true);
    FillScreen(0x0000);
}

void St7735::SetBacklight(bool on)
{
    blk_.Write(on);
}

void St7735::Reset()
{
    rst_.Write(true);
    daisy::System::Delay(10);
    rst_.Write(false);
    daisy::System::Delay(10);
    rst_.Write(true);
    daisy::System::Delay(120);
}

void St7735::WriteCommand(uint8_t cmd)
{
    cs_.Write(false);
    dc_.Write(false);
    spi_.BlockingTransmit(&cmd, 1, 10);
    cs_.Write(true);
}

void St7735::WriteData(uint8_t data)
{
    cs_.Write(false);
    dc_.Write(true);
    spi_.BlockingTransmit(&data, 1, 10);
    cs_.Write(true);
}

void St7735::WriteData16(uint16_t data)
{
    uint8_t buf[2] = {static_cast<uint8_t>(data >> 8), static_cast<uint8_t>(data & 0xFF)};
    cs_.Write(false);
    dc_.Write(true);
    spi_.BlockingTransmit(buf, 2, 10);
    cs_.Write(true);
}

void St7735::InitSequence()
{
    WriteCommand(SWRESET);
    daisy::System::Delay(150);

    WriteCommand(SLPOUT);
    daisy::System::Delay(255);

    WriteCommand(COLMOD);
    WriteData(0x05); // 16-bit

    ApplyRotation();

    WriteCommand(INVOFF);
    WriteCommand(NORON);
    daisy::System::Delay(10);
    WriteCommand(DISPON);
    daisy::System::Delay(100);
}

void St7735::ApplyRotation()
{
    WriteCommand(MADCTL);
    // MV | MX — landscape, 160 wide x 128 tall
    WriteData(0xA0);

    WriteCommand(CASET);
    WriteData(0x00);
    WriteData(0x00 + kColOffset);
    WriteData(0x00);
    WriteData(0x00 + kColOffset + kWidth - 1);

    WriteCommand(RASET);
    WriteData(0x00);
    WriteData(0x00 + kRowOffset);
    WriteData(0x00);
    WriteData(0x00 + kRowOffset + kHeight - 1);
}

void St7735::SetAddrWindow(int x, int y, int w, int h)
{
    const int x0 = x + kColOffset;
    const int x1 = x + w - 1 + kColOffset;
    const int y0 = y + kRowOffset;
    const int y1 = y + h - 1 + kRowOffset;

    WriteCommand(CASET);
    WriteData(x0 >> 8);
    WriteData(x0 & 0xFF);
    WriteData(x1 >> 8);
    WriteData(x1 & 0xFF);

    WriteCommand(RASET);
    WriteData(y0 >> 8);
    WriteData(y0 & 0xFF);
    WriteData(y1 >> 8);
    WriteData(y1 & 0xFF);

    WriteCommand(RAMWR);
}

void St7735::WritePixels(const uint16_t* data, size_t count)
{
    static uint8_t bytes[512];
    size_t         offset = 0;

    while(offset < count)
    {
        const size_t chunk = (count - offset > 256) ? 256 : (count - offset);
        for(size_t i = 0; i < chunk; ++i)
        {
            const uint16_t c = data[offset + i];
            bytes[i * 2]     = static_cast<uint8_t>(c >> 8);
            bytes[i * 2 + 1] = static_cast<uint8_t>(c & 0xFF);
        }

        cs_.Write(false);
        dc_.Write(true);
        spi_.BlockingTransmit(bytes, chunk * 2, 100);
        cs_.Write(true);

        offset += chunk;
    }
}

void St7735::FillScreen(uint16_t color)
{
    for(size_t i = 0; i < kWidth * kHeight; ++i)
        framebuffer_[i] = color;

    SetAddrWindow(0, 0, kWidth, kHeight);
    WritePixels(framebuffer_, kWidth * kHeight);
}

} // namespace rools
