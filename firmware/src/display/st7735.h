#pragma once

#include "per/gpio.h"
#include "per/spi.h"

#include <cstddef>
#include <cstdint>

namespace rools {

// ST7735S 1.77" TFT 驱动（4-wire SPI）
//
// 接线（见 board/pins.h）：
//   SCK / MOSI  → SPI 时钟和数据（只写，无 MISO）
//   CS          → 片选，低电平选中屏
//   DC (RS)     → 0=发命令，1=发数据/像素
//   RST         → 硬件复位
//   BLK         → 背光（与显示内容无关，只控制 LED 亮灭）
//
// 上层 Gfx 先画到 framebuffer_，Flush() 时整屏刷出。
class St7735 {
public:
    static constexpr int kWidth  = 160;
    static constexpr int kHeight = 128;

    St7735() = default;

    void Init();
    void SetBacklight(bool on);

    // 设定写入区域并发出 RAMWR，后续 SPI 字节按 RGB565 顺序写入显存
    void SetAddrWindow(int x, int y, int w, int h);
    void WritePixels(const uint16_t* data, size_t count);
    bool StartWriteFramebufferRect(int x, int y, int w, int h);
    bool IsBusy() const { return dma_busy_; }
    void FillScreen(uint16_t color);

    uint16_t* framebuffer() { return framebuffer_; }

private:
    static void OnDmaStart(void* context);
    static void OnDmaDone(void* context, daisy::SpiHandle::Result result);

    void Reset();
    void WriteCommand(uint8_t cmd);
    void WriteData(uint8_t data);
    void WriteData(const uint8_t* data, size_t len);
    void InitSequence();
    void ApplyRotation();
    bool StartNextDmaChunk();
    void FinishDmaTransfer();

    daisy::GPIO cs_;
    daisy::GPIO      dc_;
    daisy::GPIO      rst_;
    daisy::GPIO      blk_;

    // CPU 侧帧缓冲，RGB565（每像素 2 字节）
    uint16_t framebuffer_[kWidth * kHeight];
    static constexpr size_t kMaxTxBytes  = kWidth * kHeight * 2;
    static constexpr size_t kDmaChunkLen = kMaxTxBytes;
    uint8_t                 tx_buffer_[kMaxTxBytes];
    size_t                  tx_size_      = 0;
    size_t                  tx_offset_    = 0;
    volatile bool           dma_busy_     = false;

    // 玻璃物理 128×160，横屏使用时逻辑 160×128
    // 部分模块玻璃比可见区大，需 offset 修正偏移；本屏为 0
    static constexpr int kColOffset = 0;
    static constexpr int kRowOffset = 0;

    // MADCTL 0x36：控制行列扫描方向与 RGB/BGR 顺序
    // 0x60 = MV(行列交换,横屏) | MX(水平镜像)
    // 花屏/镜像不对时可试 0xA0、0x68、0xC8 等组合
    static constexpr uint8_t kMadctlLandscape = 0x60;
};

/** 掉电路径：仅 GPIO 关背光，禁止 SPI。Init 前调用无效。 */
void EmergencyBacklightOff();

} // namespace rools
