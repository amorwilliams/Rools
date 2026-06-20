# 01 — 产品规格

## 外形

| 项目 | Rools Core | Rools Exp（可选） |
|------|------------|------------------|
| 宽度 | 10 HP（~50.3 mm） | +2 HP（~10.16 mm） |
| 高度 | 3U（128.5 mm） | 同左 |
| 深度 | 目标 35–40 mm | 同核心 PCB |
| PCB | **4 层 1.6 mm** | [ADR-014](decisions/ADR-014-pcb-4-layer.md) |
| 面板 | 2 mm 阳极氧化铝 | 同左 |

## 显示

| 项目 | 规格 |
|------|------|
| 尺寸 | 1.77" IPS TFT |
| 方向 | 横屏（Landscape） |
| 分辨率 | 160 × 128 |
| 驱动 | ST7735（SPI） |
| Outline | 34.0 × 43.8 mm |

## 控件

| 控件 | 数量 |
|------|------|
| Encoder + Push | 2（Enc A / Enc B） |
| 小按键（6 mm tact） | 1（Enc 中间） |
| Potentiometer | 4（K1–K4） |

## I/O 汇总

| 类型 | 数量 |
|------|------|
| CV In | 4（CV1–CV4，DC）+ 4 Knob（K1–K4）分路 ADC，固件合成 |
| CV Out | 4（A–D，外置 MCP4728） |
| Audio In | 2（IN_L / IN_R，AC/DC 可选） |
| Audio Out | 2（OUT_L / OUT_R，AC/DC 可选） |
| MIDI | IN + OUT（Exp） |
| USB-A Host | 1（Exp） |

## 电气（预估，待实测）

| 项目 | 预估值 |
|------|--------|
| +12V | 150–200 mA（Core）；+50–100 mA（Exp USB Host） |
| -12V | Seed 板载音频负轨（TL074 ±10V 调理） |
| +5V | **模块内部 Buck 自 +12V 产生**（不用机箱 +5V；禁线性 7805 主路径） |

电流预算与 USB 限流见 [ADR-011](decisions/ADR-011-power-supply.md)。

## MCU

| 项目 | 规格 |
|------|------|
| 模块 | Daisy Seed |
| MCU | STM32H750，480 MHz |
| RAM | 64 MB SDRAM |
| Flash | 8 MB |
| 音频 | 48 kHz / 24-bit（Seed 板载 Codec + TL074，[ADR-016](decisions/ADR-016-onboard-audio-no-pcm3060.md)） |

## 功能路线图

| 优先级 | App |
|--------|-----|
| P0 | Spectrum Analyzer、Oscilloscope |
| P1 | Lissajous、Delay、Reverb |
| P2 | Granular、Envelope Follower |
