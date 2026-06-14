# ADR-012: 显示 SPI 与 DMA 刷新

## 状态

已接受

## 背景

ST7735 横屏 160×128 RGB565：

- 全帧 ≈ **40 KB** SPI 数据
- 目标 UI **30 fps**（频谱 / 示波）
- 音频 **48 kHz** ISR 必须实时；**禁止在 audio callback 内 SPI 绘制**

M1 固件 `st7735.cpp` 使用 `BlockingTransmit` + 全屏 `Flush()`，breadboard 可接受；量产 UI 与 FX 并发时会占满主循环时间，并可能间接影响系统实时性。

## 决策

1. **线程模型**（已实现原则，保持）：
   - `audio_callback`：仅 DSP，无 SPI / malloc / blocking
   - `ui_draw`：主循环 ~30 fps，写 framebuffer
2. **SPI 传输**：M2 前升级为 **SPI DMA**（libDaisy `SpiHandle` DMA 或等价 HAL），CPU 提交 buffer 后返回。
3. **局部刷新**：频谱 / 示波只更新脏矩形（柱条区域 + 顶栏），避免每帧 40 KB 全屏。
4. **优先级**：DMA 传输期间 CPU 不得 busy-wait 整帧；可用双 buffer + 传输完成回调链式提交下一块。
5. **M1 例外**：breadboard 阶段允许 blocking SPI；M2 软件清单将 DMA 列为 **P0**。

### 数据流（目标）

```
ui_draw() → 写 framebuffer → FlushDirty(rect)
                              ↓
                         DMA SPI → ST7735
                              ↑
                    （与 audio ISR 无交集）
```

## 理由

- 40 KB × 30 fps ≈ 1.2 MB/s SPI + CPU 打包开销；blocking 模式在 H7 上勉强，FX App 加入后不够
- libDaisy 其他外设（ADC、UART）已广泛使用 DMA；SPI 有先例可参考
- 局部刷新在 M1 计划已预留 `SetAddrWindow()` API

## 后果

- `display/st7735.*` 需重构：DMA 状态机、`FlushRect()` 替代全屏 `Flush()`
- `spectrum_view` 可标记 dirty 列，减少带宽
- 文档 [05-software-architecture.md](../05-software-architecture.md) UIRenderer 行引用本 ADR

## 待验证

- [ ] DMA 刷新时测 audio callback 最大执行时间（应无劣化）
- [ ] 30 fps 频谱 + Delay 同时运行的 CPU 占用
- [ ] SPI 时钟上限（当前 PS_8 预分频，可酌情调快）
