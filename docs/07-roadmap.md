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

- [ ] Daisy Seed + libDaisy 工具链
- [ ] ST7735 1.77" 横屏驱动
- [ ] 外置 Codec breadboard（或 Patch 子板参考）
- [ ] FFT 频谱显示 30 fps
- [ ] Enc A/B + 单键 UI 框架

## M2 硬件清单

- [ ] KiCad 原理图 / PCB
- [ ] 10HP 面板 Gerber
- [ ] 2HP Exp 面板（可选）
- [ ] MCP4728 CV Out 校准
- [ ] IN_R normalled 验证
- [ ] 功耗实测

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

见 [docs/decisions/](decisions/) ADR 与各文档「待验证」项。

## 版本命名

- 固件：`rools_firmware_vX.Y.Z.bin`
- 文档：随 Git 版本
