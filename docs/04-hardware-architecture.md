# 04 — 硬件架构

## 系统框图

```
Front Panel (10HP)
├── 1.77" IPS ST7735 ── SPI ──► Daisy Seed
├── Enc A/B + Btn ────────────► GPIO
├── K1–K4 ──► MCP6004 ──► ADC4–7 (Knob)
├── CV1–4 Jacks ──► MCP6004 ──► ADC0–3 (CV)
├── CV A–D Jacks ◄── MCP4728 ◄── I2C
└── IN/OUT Audio ──► TL074 ±10V ──► Seed pin 16–19 (板载 Codec)

Exp Panel (+2HP)
├── MIDI IN/OUT ── UART (D13–14) ──► Seed
└── USB-A Host ── OTG (D29–30) ──► Seed (+ 5V 供电)

Power: Eurorack ±12V → **Buck +5V** → Seed / Display / USB VBUS（限流）
       详见 [ADR-011](decisions/ADR-011-power-supply.md)
```

## 音频路径（无 PCM3060）

Eurorack jack ↔ **TL074**（×0.1 入 / ×10 出）↔ Seed **AUDIO_IN/OUT_L/R**（pin 16–19，板载 WM8731）。详见 [ADR-016](decisions/ADR-016-onboard-audio-no-pcm3060.md)。

## IN_R Normalled

IN_R jack **normalize 到 IN_L**，KiCad 网 **`IN_L_NORM`**（`J_CV_5` Tip ↔ `J_CV_6` Switch）。空插 R 时右声道 = 左声道。

接法详见 [ADR-010](decisions/ADR-010-mono-stereo-normaling.md)。

## Daisy Seed 引脚预算（2026-06 定稿）

**左侧 pin 1–19**

| 功能 | Daisy | Seed pin |
|------|-------|----------|
| （NC 预留） | D0 | 1 |
| Enc A A/B/Sw | D1–D3 | 2–4 |
| Btn | D4 | 5 |
| 显示屏 SPI | D5–D10 | 6–11 |
| I2C（MCP4728） | D11–D12 | 12–13 |
| USART1（Exp MIDI） | D13–D14 | 14–15 |
| Audio L/R In/Out | — | 16–19 |

**右侧 pin 22–32**

| 功能 | Daisy | Seed pin | ADC |
|------|-------|----------|-----|
| CV1–4 | D15–D18 | 22–25 | 0–3 |
| KNOB1–4 | D19–D22 | 26–29 | 4–7 |
| Enc B A/B/Sw | D23–D25 | 30–32 | — |
| SAI2 / PCM3060 | D26–D27 | 33–34 | NC |
| 掉电 EXTI | D28 | 35 | net `EXTI_PWR`（Seed 页已接；比较器/保持电容页 **待画**，ADR-015） |

## ADC 分配

| 通道 | 用途 |
|------|------|
| ADC 0–3 | CV1–CV4（jack） |
| ADC 4–7 | KNOB1–K4 |
| ADC 8+ | — | D28 作 GPIO EXTI，非 ADC 用途 |

固件在 AudioCallback 合成 `ControlColumn.sum`（[ADR-017](decisions/ADR-017-split-adc-knob-cv-sum.md)）。

## DAC

| 来源 | 用途 |
|------|------|
| MCP4728 ×4 | CV Out A–D |
| Seed 内置 DAC ×2 | v1 闲置 |

## 深度与机械

- 目标深度 **35 mm**（对齐 O_C 4.1）
- 1.77" 屏 + USB-A 为深度主要约束
- Core + Exp 同 PCB 侧出或 FFC 连接（KiCad 阶段定）
- 机械方案（Seed 贴片 / 沉板 USB / 矮排针）见 [ADR-013](decisions/ADR-013-mechanical-depth.md)

## PCB

- **4 层 / 1.6 mm**（[ADR-014](decisions/ADR-014-pcb-4-layer.md)）：L2 完整地、L3 电源；数字/模拟分区
- KiCad 工程后续置于 [hardware/](../hardware/)

## BOM 关键件（见 hardware/bom/bom-draft.md）

- Daisy Seed
- ST7735 1.77" IPS
- TL074 + MCP6004（音频/CV 调理）
- MCP4728
- USB-A 母座 + 5V Host 供电/限流（Exp）
- Thonkiconn 或 PJ398 插孔 ×12
