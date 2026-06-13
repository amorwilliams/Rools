# 04 — 硬件架构

## 系统框图

```
Front Panel (10HP)
├── 1.77" IPS ST7735 ── SPI ──► Daisy Seed
├── Enc A/B + Btn ────────────► GPIO
├── K1–K4 ──► CV Front ──► ADC (×4, 与 CV jack 求和)
├── CV1–4 Jacks ──► CV Front
├── CV A–D Jacks ◄── MCP4728 ◄── I2C
└── IN/OUT Audio ──► Codec ◄── I2S ──► Seed

Exp Panel (+2HP)
├── MIDI IN/OUT ── UART ──► Seed
└── USB-A Host ── OTG ──► Seed (+ 5V 供电)

Power: Eurorack ±12V → 5V/3.3V → Seed / Codec / Display
```

## 为何外置 Audio Codec

Daisy Seed onboard 音频为 line level ±1.8V AC 耦合。Eurorack 需要 ±10V DC 耦合，参考 [Daisy Patch](https://daisy.audio/hardware/Patch/) 使用 PCM3060 类 Codec + 运放调理。

## IN_R Normalled

IN_R jack **normalize 到 IN_L**（ switching jack 或 PCB 飞线方案，KiCad 阶段定稿）。空插 R 时 Codec 右声道收到左声道信号。

## Daisy Seed 引脚预算（概念）

| 功能 | 引脚 | 类型 |
|------|------|------|
| 显示屏 SPI | 5 | D |
| Enc A | 3 | D (A/B/Click) |
| Enc B | 3 | D |
| Btn | 1 | D |
| CV1–4 + K1–4 | 4 | ADC |
| Audio Codec I2S | 4–6 | SAI |
| Codec I2C | 2 | I2C |
| MCP4728 | 2 | I2C（与 Codec 共总线） |
| MIDI UART | 2 | D |
| USB OTG | 差分 + 5V 控 | Host |

## ADC 分配

| 通道 | 用途 |
|------|------|
| ADC 1–4 | CV1+K1 … CV4+K4 求和 |
| ADC 5–12 | 预留（Exp v2 / 校准） |

## DAC

| 来源 | 用途 |
|------|------|
| MCP4728 ×4 | CV Out A–D |
| Seed 内置 DAC ×2 | v1 闲置 |

## 深度与机械

- 目标深度 **35 mm**（对齐 O_C 4.1）
- 1.77" 屏 + 竖插 USB-A 为深度主要约束
- Core + Exp 同 PCB 侧出或 FFC 连接（KiCad 阶段定）

## BOM 关键件（见 hardware/bom/bom-draft.md）

- Daisy Seed
- ST7735 1.77" IPS
- PCM3060（或同类）+ 运放
- MCP4728
- USB-A 母座 + 5V Host 供电/限流
- Thonkiconn 或 PJ398 插孔 ×12
