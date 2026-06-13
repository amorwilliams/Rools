# 00 — 项目概述

## 是什么

**Rools** 是一款 Eurorack 多功能模块：

- **10HP 核心**：大屏频谱/波形可视化 + DSP 效果器平台
- **+2HP 可选扩展**：MIDI + USB-A Host（样本/预设/固件更新）
- **MCU**：Electrosmith [Daisy Seed](https://daisy.audio/software/)

## 设计目标

1. **可视化优先** — 1.77" IPS 横屏，频谱/示波/李萨如
2. **平台化** — 统一 UI/I/O，单 App 模式增量扩展功能
3. **紧凑** — 10HP 核心对标 O_C 功能面宽度，大屏差异化

## 参考模块

| 模块 | 借鉴 |
|------|------|
| [O'Tools Plus](https://www.jonesvideo.com/otool/Otool_Plus_UserManual.pdf) | 测量/可视化 |
| [ALM MFX](https://www.busycircuits.com/alm017/) | 立体声 DSP FX |
| [O_C 4.1](https://modulargrid.net/e/tunefish-modular-ornament-crime-o-c-t4-1-o-r-n-8) | 10HP+2HP 架构、双 Encoder |
| [Instruō arbhar / Lúbadh](https://www.instruomodular.com/firmware/) | USB 样本/预设/OTA |

## 运行模式

**单 App**：一次运行一个功能（Spectrum / Delay / …），切换需退出当前 App。

## 术语

| 术语 | 含义 |
|------|------|
| HP | Horizontal Pitch，1 HP = 5.08 mm（面板宽约 50.3 mm @ 10HP） |
| CV | Control Voltage，控制电压 |
| Normalled | 插孔未接线时内部连通到另一孔（如 IN_R → IN_L） |
| App | 固件内一个独立功能模块 |
