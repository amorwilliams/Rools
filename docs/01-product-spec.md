# 01 — 产品规格

## 外形

| 项目 | Rools Core | Rools Exp（可选） |
|------|------------|------------------|
| 宽度 | 10 HP（~50.3 mm） | +2 HP（~10.16 mm） |
| 高度 | 3U（128.5 mm） | 同左 |
| 深度 | 目标 35–40 mm | 同核心 PCB |
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
| CV In | 4（CV1–CV4，DC，Knob 求和） |
| CV Out | 4（A–D，外置 MCP4728） |
| Audio In | 2（IN_L / IN_R，AC/DC 可选） |
| Audio Out | 2（OUT_L / OUT_R，AC/DC 可选） |
| MIDI | IN + OUT（Exp） |
| USB-A Host | 1（Exp） |

## 电气（预估，待实测）

| 项目 | 预估值 |
|------|--------|
| +12V | 150–200 mA（Core）；+50–100 mA（Exp USB Host） |
| -12V | 视 Codec 负轨需求 |
| +5V | 内部产生 |

## MCU

| 项目 | 规格 |
|------|------|
| 模块 | Daisy Seed |
| MCU | STM32H750，480 MHz |
| RAM | 64 MB SDRAM |
| Flash | 8 MB |
| 音频 | 48 kHz / 24-bit（经外置 Codec，libDaisy 默认） |

## 功能路线图

| 优先级 | App |
|--------|-----|
| P0 | Spectrum Analyzer、Oscilloscope |
| P1 | Lissajous、Delay、Reverb |
| P2 | Granular、Envelope Follower |
