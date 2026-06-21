# 04 — 硬件架构

## 系统框图

```
Front Panel (10HP)
├── 1.77" IPS ST7735 ── SPI ──► Daisy Seed
├── Enc A/B + Btn ────────────► GPIO
├── K1–K4 ──► MCP6004 ──► ADC4–7 (Knob)
├── CV1–4 Jacks ──► MCP6004 ──► ADC0–3 (CV)
├── CV A–D Jacks ◄── OPA4171 ◄── DAC8565 ◄── SPI1
└── IN/OUT Audio ──► TL074 ±10V ──► Seed pin 16–19 (板载 Codec)

Exp Panel (+2HP)
├── MIDI IN/OUT — USART1（D13/D14，Seed pin 14/15）→ P2 pin 7/8
└── USB-A Host ── OTG (D29–30, D0 ID) ──► P2 `+5V` + Exp 本地 VBUS 限流

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
| Exp `EXP_GPIO0` | D0 | 1 | `USB_HS_ID` → P2 pin 15 |
| Enc A A/B/Sw | D1–D3 | 2–4 |
| Btn | D4 | 5 |
| 显示屏 SPI | D5–D10 | 6–11 | D8/D10 与 DAC8565 共用 |
| Exp I2C1 | D11–D12 | 12–13 | `I2C_SCL` / `I2C_SDA` → P2 pin 13/14 |
| Exp MIDI | D13–D14 | 14–15 | `MIDI_TX` / `MIDI_RX`（USART1）→ P2 pin 7/8 |
| Audio L/R In/Out | — | 16–19 |

**右侧 pin 22–32**

| 功能 | Daisy | Seed pin | ADC |
|------|-------|----------|-----|
| CV1–4 | D15–D18 | 22–25 | 0–3 |
| KNOB1–4 | D19–D22 | 26–29 | 4–7 |
| Enc B A/B/Sw | D23–D25 | 30–32 | — |
| DAC8565 | D26–D27 | 33–34 | `DAC_CS` / `DAC_LDAC` |
| 掉电检测 | D28 | 35 | net `EXTI_PWR` ← LM393（ADR-015） |

## ADC 分配

| 通道 | 用途 |
|------|------|
| ADC 0–3 | CV1–CV4（jack） |
| ADC 4–7 | KNOB1–K4 |
| ADC 8+ | — | D28 作掉电检测 GPIO，非 ADC 用途 |

固件在 AudioCallback 合成 `ControlColumn.sum`（[ADR-017](decisions/ADR-017-split-adc-knob-cv-sum.md)）。

## DAC

| 来源 | 用途 |
|------|------|
| DAC8565 ×4 | CV Out A–D（内部 2.5V 基准 → OPA4171 0–2.5V → ±10V；R 12.5k/100k 0.1%） |
| Seed 内置 DAC ×2 | v1 闲置 |

## P2 EXP_BUS（2×8，Core ↔ Exp）

| Pin | 信号 | 说明 |
|-----|------|------|
| 1 | `+3V3_D` | 数字域 |
| 2 | `+3V3_A` | Seed 模拟域（MIDI 光耦等） |
| 3 | `SWDIO` | 与 P3 并联，Exp 侧可选接 |
| 4 | `SWCLK` | |
| 5 | `RESET` | |
| 6 | `GND` | |
| 7 | `MIDI_TX` | D13 / Seed pin 14（USART1_TX） |
| 8 | `MIDI_RX` | D14 / Seed pin 15（USART1_RX） |
| 9 | `USB_D-` | D29 |
| 10 | `USB_D+` | D30 |
| 11 | `+5V` | Buck 输出；Exp USB Host 本地限流开关 |
| 12 | `GND` | |
| 13 | `I2C_SCL` | D11；Core 侧 4.7k→`+3V3_D` |
| 14 | `I2C_SDA` | D12 |
| 15 | `EXP_GPIO0` | D0 / `USB_HS_ID`；Exp Host 模式接 GND |
| 16 | `GND` | |

## 掉电检测

`+12V` → R43/R64 分压 → **LM393 U10A**（基准 `+3V3_D`）→ net **`EXTI_PWR`** → Seed **D28**。`+12V_SAFE`（D3 + C36/C37 ~2000 µF）维持 hold-up。阈值 ~10.6 V；固件 1 kHz 采样 debounce（连续 LOW ≥5 ms）→ 关背光 → Flash `Flush()`。详见 [ADR-015](decisions/ADR-015-settings-persistence.md)。

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
- TL074 + MCP6004（音频/CV In 调理）
- DAC8565 + OPA4171（CV Out）
- USB-A 母座 + 5V Host 供电/限流（Exp）
- Thonkiconn 或 PJ398 插孔 ×12
