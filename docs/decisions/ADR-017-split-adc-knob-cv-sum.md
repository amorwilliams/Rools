# ADR-017: CV / Knob 分路 ADC + 软件合成

## 状态

已接受

## 背景

原规格写 K1–K4 与 CV1–4 **硬件求和**后接 4 路 ADC（Patch 式）。Core 原理图改为 **8 路独立 ADC**（4 CV + 4 Knob），固件在 AudioCallback 合成 `ControlColumn.sum`。

## 决策

| 路 | Seed GPIO | Seed pin | 用途 |
|----|-----------|----------|------|
| CV1–4 | D15–D18 | 22–25 | CV jack（MCP6004 调理） |
| KNOB1–4 | D19–D22 | 26–29 | K1–K4 pot |
| Enc B | D23–D25 | 30–32 | 不占 ADC |

固件：

- `AdcChannelConfig` ×8，顺序 CV1…4、KNOB1…4
- `knob`：ADC uni 0…1
- `cv`：`CvAdcToNormalized()`（±10V 标定）
- `sum`：`ClampBipolar(cv + (knob - 0.5f) * 2.f)`（M2 可调权重）

## 理由

- 原理图分路更清晰；校准可独立
- 仍暴露单列 `knob` / `cv` 供 App 选用

## 后果

- 修订 [03-io-spec.md](../03-io-spec.md)、[04-hardware-architecture.md](../04-hardware-architecture.md)
- [calibration_app](firmware) 仍仅 CV 四路；Knob trim 可后加
