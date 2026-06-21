# 03 — I/O 规格

## 拓扑

**Core**：6 模拟输入（4 CV + 2 Audio）+ 6 模拟输出（4 CV + 2 Audio）  
**Exp**：MIDI ×2 + USB-A Host

## CV 输入（CV1–CV4）

| 项目 | 规格 |
|------|------|
| 方向 | 输入 |
| 耦合 | DC 固定 |
| 电压 | Eurorack ±10V（调理至 ADC 量程） |
| Knob | K1–K4 **分路 ADC**（D19–D22）；`sum` 固件合成（[ADR-017](decisions/ADR-017-split-adc-knob-cv-sum.md)） |
| 默认语义 | K1/K2 主参数；K3 Clock；K4 Trig（App 可重映射） |

## CV 输出（A–D）

| 项目 | 规格 |
|------|------|
| 方向 | 输出 |
| 耦合 | DC 固定 |
| 硬件 | DAC8565（SPI，4× 16-bit，内部 2.5V 基准）+ OPA4171 0–2.5V→±10V |
| 用途 | LFO / 包络 / Trig / Gate CV（App 分配） |

## 音频输入（IN_L / IN_R）

| 项目 | 规格 |
|------|------|
| 方向 | 输入 |
| 耦合 | 硬件 DC；软件 AC/DC 可选 |
| 电压 | Eurorack ±10V |
| **Mono** | **IN_R normalled 到 IN_L**（R 孔空 = R 通道 = L） |
| Codec | **Seed 板载**（pin 16–19）+ TL074 ±10V |

## 音频输出（OUT_L / OUT_R）

| 项目 | 规格 |
|------|------|
| 方向 | 输出 |
| 耦合 | 硬件 DC；软件 AC/DC 可选 |
| **Mono→立体声** | 软件 `out_R = out_L` |
| 菜单 | Input: Auto / Mono / Stereo；Output: Stereo / Mono(L→R) |

## 立体声 / Mono 实现分层

| 环节 | 层 | 做法 |
|------|-----|------|
| 只插 IN_L | 硬件 | IN_R normalled → IN_L |
| L 复制到 OUT_R | 软件 DSP | AudioEngine callback |

## Exp — MIDI

| 项目 | 规格 |
|------|------|
| 接口 | MIDI IN / OUT |
| 物理 | 3.5 mm TRS 或 DIN（待定） |
| 协议 | UART |

## Exp — USB-A Host

| 功能 | 路径 |
|------|------|
| 样本 | `/rools/samples/` |
| 预设 | `/rools/presets/` |
| Scene | `/rools/scenes/` |
| 固件 OTA | `/rools/_updater/*.bin` → 刷完删除，写 `update.log` |

## AC/DC 切换范围

| 接口 | 软件 AC/DC |
|------|-----------|
| IN_L / IN_R | ✅ |
| OUT_L / OUT_R | ✅ |
| CV1–CV4 | ❌ DC 固定 |
| CV A–D | ❌ DC 固定 |
