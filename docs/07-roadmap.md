# 07 — 开发里程碑

## 阶段总览

| 阶段 | 目标 | 产出 |
|------|------|------|
| **M0 设计** | 设计案 + 工程结构 | 本文档集 ✅ |
| **M1 原型** | Seed + 屏 + 音频 breadboard | Spectrum App 可运行 |
| **M2 面板** | PCB + 10HP Core（+可选 Exp） | 完整 I/O |
| **M3 FX** | Delay / Reverb App | 可演出 |
| **M4 抛光** | UI 动画、预设、校准 | v1.0 |

## M1 原型清单

- [x] Daisy Seed + libDaisy 工具链
- [x] ST7735 1.77" 横屏驱动（blocking SPI，M1 可接受）
- [ ] 外置 Codec breadboard（或 Patch 子板参考）
- [ ] FFT 频谱显示 30 fps
- [x] Enc A/B + 单键 UI 框架

## M2 硬件清单

- [x] KiCad 原理图 Core 页（Power Rev 0.2 / CV / Audio / UI / Seed；[ADR-011](decisions/ADR-011-power-supply.md) Buck + 掉电检测）
- [ ] KiCad PCB Layout（[ADR-014](decisions/ADR-014-pcb-4-layer.md) **4 层**）
- [ ] 10HP 面板 Gerber（[ADR-013](decisions/ADR-013-mechanical-depth.md) 深度预算）
- [ ] 2HP Exp 面板（可选）
- [ ] DAC8565 CV Out 校准
- [ ] IN_R normalled 验证（[ADR-010](decisions/ADR-010-mono-stereo-normaling.md)）
- [ ] 功耗实测（Core / Core+Exp / USB U 盘）

## M2 软件清单

- [ ] **ST7735 SPI DMA + 局部刷新**（[ADR-012](decisions/ADR-012-display-spi-dma.md)，P0）

## M3 软件清单

- [ ] AppShell 完整实现
- [ ] **App 丝滑切换**（[08-app-switching.md](08-app-switching.md)：`request_app_switch`、fade、DspMemoryPool）
- [ ] App 菜单 UI（Enc A 长按）
- [ ] Delay / Reverb（DaisySP）
- [ ] 预设 JSON 读写（Flash）
- [ ] CV 列映射 API

## M4 清单

- [ ] USB Host + FAT32
- [ ] OTA `_updater` 流程
- [ ] Granular + 样本加载
- [ ] 出厂校准与测试脚本

## 风险跟踪

| 风险 | ADR | 阶段 |
|------|-----|------|
| +5V 发热 / USB 500 mA | [ADR-011](decisions/ADR-011-power-supply.md) | M2 PCB |
| SPI 阻塞拖慢 UI | [ADR-012](decisions/ADR-012-display-spi-dma.md) | M2 固件 |
| 深度超标 | [ADR-013](decisions/ADR-013-mechanical-depth.md) | M2 机械 |
| 2 层 PCB 噪声 / USB | [ADR-014](decisions/ADR-014-pcb-4-layer.md) | M2 Layout |
| Normalled 接错 | [ADR-010](decisions/ADR-010-mono-stereo-normaling.md) | M2 验证 |

另见 [docs/decisions/](decisions/) 与各文档「待验证」项。

## 版本命名

- 固件：`rools_firmware_vX.Y.Z.bin`
- 文档：随 Git 版本
