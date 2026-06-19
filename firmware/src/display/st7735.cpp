#include "display/st7735.h"

#include "board/pins.h"
#include "sys/system.h"

#include <cstring>

namespace rools {

namespace {
constexpr size_t kDCacheLineSize = 32;

void CleanDCacheRange(void* addr, size_t len)
{
    if(len == 0)
        return;

    uintptr_t start = reinterpret_cast<uintptr_t>(addr);
    uintptr_t end   = start + len;
    start &= ~(static_cast<uintptr_t>(kDCacheLineSize - 1));
    end = (end + (kDCacheLineSize - 1)) & ~(static_cast<uintptr_t>(kDCacheLineSize - 1));

    SCB_CleanDCache_by_Addr(reinterpret_cast<uint32_t*>(start), static_cast<int32_t>(end - start));
}

// ST7735 指令字，完整列表见数据手册 "Command List"
enum Cmd : uint8_t {
    SWRESET = 0x01, // 软件复位
    SLPOUT  = 0x11, // 退出睡眠
    NORON   = 0x13, // 正常显示模式（关闭部分省电）
    INVOFF  = 0x20, // 关闭颜色反转：0x0000=黑，0xFFFF=白
    INVON   = 0x21, // 开启颜色反转：0x0000 在屏上显示为白
    DISPON  = 0x29, // 开显示
    CASET   = 0x2A, // 列地址范围 Column Address Set
    RASET   = 0x2B, // 行地址范围 Row Address Set
    RAMWR   = 0x2C, // 写显存 Memory Write（之后跟像素数据）
    MADCTL  = 0x36, // 显存访问方向 Memory Data Access Control
    COLMOD  = 0x3A, // 像素格式，0x05 = 16bit/pixel (RGB565)
    FRMCTR1 = 0xB1, // 帧率控制（正常模式）
    FRMCTR2 = 0xB2, // 帧率控制（空闲模式）
    FRMCTR3 = 0xB3, // 帧率控制（部分模式）
    INVCTR  = 0xB4, // 显示反转控制
    PWCTR1  = 0xC0, // 电源控制 1
    PWCTR2  = 0xC1,
    PWCTR3  = 0xC2,
    PWCTR4  = 0xC3,
    PWCTR5  = 0xC4,
    VMCTR1  = 0xC5, // VCOM 电压
    GMCTRP1 = 0xE0, // 正极伽马校正
    GMCTRN1 = 0xE1, // 负极伽马校正
};

} // namespace

void St7735::Init()
{
    // --- GPIO：CS/DC/RST/BLK 由 MCU 控制，SPI 走软件片选 ---
    daisy::GPIO::Config gcfg;
    gcfg.mode = daisy::GPIO::Mode::OUTPUT;
    gcfg.pull = daisy::GPIO::Pull::NOPULL;

    gcfg.pin = pins::kLcdCs;
    cs_.Init(gcfg);
    cs_.Write(true); // CS 高 = 不选中

    gcfg.pin = pins::kLcdDc;
    dc_.Init(gcfg);

    gcfg.pin = pins::kLcdRst;
    rst_.Init(gcfg);

    gcfg.pin = pins::kLcdBlk;
    blk_.Init(gcfg);

    // SPI1 主机，仅 MOSI；PS_8 分频，过快可能横纹，可改 PS_16/32
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
    FillScreen(0x0000); // 清屏为黑（需与 INVOFF/INVON 一致）
}

void St7735::SetBacklight(bool on)
{
    // 背光 LED，高电平亮。与像素颜色无关。
    blk_.Write(on);
}

void St7735::Reset()
{
    // 硬件复位时序：高 → 低 ≥10ms → 高，再等芯片就绪
    rst_.Write(true);
    daisy::System::Delay(10);
    rst_.Write(false);
    daisy::System::Delay(10);
    rst_.Write(true);
    daisy::System::Delay(120);
}

void St7735::WriteCommand(uint8_t cmd)
{
    // SPI 写命令：DC=0，CS 拉低发 1 字节指令，CS 拉高
    cs_.Write(false);
    dc_.Write(false);
    spi_.BlockingTransmit(&cmd, 1, 10);
    cs_.Write(true);
}

void St7735::WriteData(uint8_t data)
{
    WriteData(&data, 1);
}

void St7735::WriteData(const uint8_t* data, size_t len)
{
    // SPI 写参数/像素：DC=1
    if(len == 0)
        return;
    cs_.Write(false);
    dc_.Write(true);
    spi_.BlockingTransmit(const_cast<uint8_t*>(data), len, 100);
    cs_.Write(true);
}

void St7735::InitSequence()
{
    // 以下序列来自 ST7735S 1.77" 模块厂商 init table
    // 顺序：复位 → 帧率 → 电源/VCOM → 像素格式 → 伽马 → 方向 → 开显

    WriteCommand(SWRESET);
    daisy::System::Delay(150);

    WriteCommand(SLPOUT);
    daisy::System::Delay(120);

    // 帧率：影响刷新与横纹，一般沿用厂商值
    const uint8_t frmctr1[] = {0x00, 0x3F, 0x3F};
    WriteCommand(FRMCTR1);
    WriteData(frmctr1, sizeof(frmctr1));

    const uint8_t frmctr2[] = {0x0F, 0x01, 0x01};
    WriteCommand(FRMCTR2);
    WriteData(frmctr2, sizeof(frmctr2));

    const uint8_t frmctr3[] = {0x05, 0x3C, 0x3C, 0x05, 0x3C, 0x3C};
    WriteCommand(FRMCTR3);
    WriteData(frmctr3, sizeof(frmctr3));

    WriteCommand(INVCTR);
    WriteData(0x03);

    // 电源阶梯：影响对比度、色偏
    const uint8_t pwctr1[] = {0xFC, 0x08, 0x02};
    WriteCommand(PWCTR1);
    WriteData(pwctr1, sizeof(pwctr1));

    WriteCommand(PWCTR2);
    WriteData(0xC0);

    const uint8_t pwctr3[] = {0x0D, 0x00};
    WriteCommand(PWCTR3);
    WriteData(pwctr3, sizeof(pwctr3));

    const uint8_t pwctr4[] = {0x8D, 0x2A};
    WriteCommand(PWCTR4);
    WriteData(pwctr4, sizeof(pwctr4));

    const uint8_t pwctr5[] = {0x8D, 0xEE};
    WriteCommand(PWCTR5);
    WriteData(pwctr5, sizeof(pwctr5));

    WriteCommand(VMCTR1);
    WriteData(0x0F);

    // 16-bit RGB565
    WriteCommand(COLMOD);
    WriteData(0x05);

    // 伽马曲线：亮部/暗部层次，厂商调好的 16 字节表
    const uint8_t gmctp[] = {
        0x02, 0x1C, 0x07, 0x12, 0x37, 0x32, 0x29, 0x2C,
        0x29, 0x25, 0x2B, 0x39, 0x00, 0x01, 0x03, 0x10,
    };
    WriteCommand(GMCTRP1);
    WriteData(gmctp, sizeof(gmctp));

    const uint8_t gmcntn[] = {
        0x03, 0x1D, 0x07, 0x06, 0x2E, 0x2C, 0x29, 0x2C,
        0x2E, 0x2E, 0x37, 0x3F, 0x00, 0x00, 0x02, 0x10,
    };
    WriteCommand(GMCTRN1);
    WriteData(gmcntn, sizeof(gmcntn));

    ApplyRotation();

    // INVOFF：逻辑黑=屏上黑。若白底则改 INVON，或保持 INVON 时所有颜色取反
    WriteCommand(INVOFF);
    WriteCommand(NORON);
    daisy::System::Delay(10);
    WriteCommand(DISPON);
    daisy::System::Delay(100);
}

void St7735::ApplyRotation()
{
    WriteCommand(MADCTL);
    WriteData(kMadctlLandscape);

    // CASET/RASET 设全屏可见窗口（含 offset），初始化时先限定一次
    const uint8_t caset[] = {
        0x00,
        static_cast<uint8_t>(kColOffset),
        0x00,
        static_cast<uint8_t>(kColOffset + kWidth - 1),
    };
    WriteCommand(CASET);
    WriteData(caset, sizeof(caset));

    const uint8_t raset[] = {
        0x00,
        static_cast<uint8_t>(kRowOffset),
        0x00,
        static_cast<uint8_t>(kRowOffset + kHeight - 1),
    };
    WriteCommand(RASET);
    WriteData(raset, sizeof(raset));
}

void St7735::SetAddrWindow(int x, int y, int w, int h)
{
    // 局部刷新：先告诉控制器写哪块矩形，再 RAMWR，然后连续发 w*h 个 RGB565
    const int x0 = x + kColOffset;
    const int x1 = x + w - 1 + kColOffset;
    const int y0 = y + kRowOffset;
    const int y1 = y + h - 1 + kRowOffset;

    const uint8_t caset[] = {
        static_cast<uint8_t>(x0 >> 8),
        static_cast<uint8_t>(x0 & 0xFF),
        static_cast<uint8_t>(x1 >> 8),
        static_cast<uint8_t>(x1 & 0xFF),
    };
    WriteCommand(CASET);
    WriteData(caset, sizeof(caset));

    const uint8_t raset[] = {
        static_cast<uint8_t>(y0 >> 8),
        static_cast<uint8_t>(y0 & 0xFF),
        static_cast<uint8_t>(y1 >> 8),
        static_cast<uint8_t>(y1 & 0xFF),
    };
    WriteCommand(RASET);
    WriteData(raset, sizeof(raset));

    WriteCommand(RAMWR);
}

void St7735::WritePixels(const uint16_t* data, size_t count)
{
    // RGB565 高字节在前（MSB first），与 COLMOD 0x05 一致
    // 分块发送：避免单次 SPI buffer 过大，每块最多 512 像素
    static uint8_t bytes[1024];
    size_t         offset = 0;
    cs_.Write(false);
    dc_.Write(true);

    while(offset < count)
    {
        const size_t chunk = (count - offset > 512) ? 512 : (count - offset);
        for(size_t i = 0; i < chunk; ++i)
        {
            const uint16_t c = data[offset + i];
            bytes[i * 2]     = static_cast<uint8_t>(c >> 8);
            bytes[i * 2 + 1] = static_cast<uint8_t>(c & 0xFF);
        }

        spi_.BlockingTransmit(bytes, chunk * 2, 100);
        offset += chunk;
    }
    cs_.Write(true);
}

bool St7735::StartWriteFramebufferRect(int x, int y, int w, int h)
{
    if(w <= 0 || h <= 0 || IsBusy())
        return false;

    int x0 = x;
    int y0 = y;
    int x1 = x + w;
    int y1 = y + h;
    if(x0 < 0)
        x0 = 0;
    if(y0 < 0)
        y0 = 0;
    if(x1 > kWidth)
        x1 = kWidth;
    if(y1 > kHeight)
        y1 = kHeight;
    if(x1 <= x0 || y1 <= y0)
        return false;

    const int rect_w = x1 - x0;
    const int rect_h = y1 - y0;
    tx_size_         = static_cast<size_t>(rect_w * rect_h * 2);
    if(tx_size_ == 0 || tx_size_ > kMaxTxBytes)
        return false;

    size_t out = 0;
    for(int row = y0; row < y1; ++row)
    {
        const uint16_t* src = &framebuffer_[row * kWidth + x0];
        for(int col = 0; col < rect_w; ++col)
        {
            const uint16_t c    = src[col];
            tx_buffer_[out++]   = static_cast<uint8_t>(c >> 8);
            tx_buffer_[out++]   = static_cast<uint8_t>(c & 0xFF);
        }
    }

    SetAddrWindow(x0, y0, rect_w, rect_h);
    tx_offset_ = 0;
    dma_busy_  = true;
    cs_.Write(false);
    dc_.Write(true);
    if(!StartNextDmaChunk())
    {
        FinishDmaTransfer();
        return false;
    }
    return true;
}

bool St7735::StartNextDmaChunk()
{
    if(tx_offset_ >= tx_size_)
    {
        FinishDmaTransfer();
        return true;
    }

    const size_t remaining = tx_size_ - tx_offset_;
    const size_t chunk     = (remaining > kDmaChunkLen) ? kDmaChunkLen : remaining;
    CleanDCacheRange(tx_buffer_ + tx_offset_, chunk);
    auto result = spi_.DmaTransmit(tx_buffer_ + tx_offset_, chunk, OnDmaStart, OnDmaDone, this);
    if(result != daisy::SpiHandle::Result::OK)
        return false;

    tx_offset_ += chunk;
    return true;
}

void St7735::FinishDmaTransfer()
{
    cs_.Write(true);
    dma_busy_ = false;
}

void St7735::OnDmaStart(void* context)
{
    (void)context;
}

void St7735::OnDmaDone(void* context, daisy::SpiHandle::Result result)
{
    St7735* self = static_cast<St7735*>(context);
    if(!self)
        return;

    if(result != daisy::SpiHandle::Result::OK)
    {
        self->FinishDmaTransfer();
        return;
    }

    if(!self->StartNextDmaChunk())
        self->FinishDmaTransfer();
}

void St7735::FillScreen(uint16_t color)
{
    for(size_t i = 0; i < kWidth * kHeight; ++i)
        framebuffer_[i] = color;

    SetAddrWindow(0, 0, kWidth, kHeight);
    WritePixels(framebuffer_, kWidth * kHeight);
}

} // namespace rools
