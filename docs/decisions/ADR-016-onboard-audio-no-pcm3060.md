# ADR-016: 板载音频（无 PCM3060）

## 状态

已接受

## 背景

原计划参考 Daisy Patch 使用外置 PCM3060 + SAI2。Core 原理图定稿改为 **Seed 板载 Codec**（pin 16–19）+ **TL074** Eurorack ±10V 调理，不装 PCM3060。

## 决策

1. 音频 I/O 走 Seed **AUDIO_IN/OUT_L/R**（板载 WM8731 路径，libDaisy 默认 `StartAudio`）。
2. Eurorack jack ↔ TL074（×0.1 入 / ×10 出）↔ Seed 16–19。
3. **不**使用 SAI2（D26–27 等）接外置 Codec；D11–12 **NC**（CV Out 走 SPI1 DAC8565）。
4. IN_R **normalled** 经 `IN_L_NORM` 网（见 [ADR-010](ADR-010-mono-stereo-normaling.md)）。

## 理由

- 少一颗 Codec，BOM/布局更简单
- Seed 48 kHz 板载路径与现有固件 `AudioHandle` 一致
- Patch 的 ±10V 运放调理仍适用

## 后果

- Eurorack **jack 侧** DC ±10V；Seed 引脚边界仍为 **AC 耦合**（见 [ADR-004](ADR-004-ac-dc-coupling.md)）
- 示波器绝对 DC 电压测量优先用 CV ADC，非音频口
- 文档/BOM 删除 PCM3060；`kAudioFullScaleVolts` 按 ±1.8V line level 标定

## 待验证

- [ ] 实机：音频电平与噪声（M2）
- [ ] IN_R 空插时 R 通道 = L（示波器）
