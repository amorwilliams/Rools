#include "board/spi1_bus.h"

#include "board/pins.h"

namespace rools {

daisy::SpiHandle Spi1Bus::handle_;
bool             Spi1Bus::inited_ = false;

daisy::SpiHandle::Result Spi1Bus::InitDefault()
{
    if(inited_)
        return daisy::SpiHandle::Result::OK;

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

    const auto result = handle_.Init(scfg);
    inited_           = (result == daisy::SpiHandle::Result::OK);
    return result;
}

daisy::SpiHandle& Spi1Bus::Handle()
{
    return handle_;
}

bool Spi1Bus::IsReady()
{
    return inited_;
}

} // namespace rools
