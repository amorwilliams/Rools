# ADR-004: AC/DC 耦合策略

## 状态

已接受

## 决策

| 接口 | 硬件 | 软件 |
|------|------|------|
| IN_L/R, OUT_L/R | **Eurorack jack 侧** DC ±10V（TL074）；Seed pin 16–19 边界 **AC 耦合** | AC/DC 可选（IIR 高通 ~1–20 Hz） |
| CV1–CV4, CV A–D | DC 耦合 | 固定 DC |

CV 输出必须 DC（1V/oct、包络等）。音频 FX 默认软件 AC；示波/频谱默认 DC。

## 理由

- 8HP/10HP 无空间做 per-jack 物理 AC/DC 开关
- O'Tools 类测量需 DC 输入
- 软件高通满足 FX 去 DC 偏置

## 后果

- `CouplingMode` 在 AudioEngine 与 App 默认值中实现
- 不做面板耦合开关

## 实现提醒（Seed 板载 + TL074）

- Eurorack **jack 侧**经 TL074 为 DC ±10V（[ADR-016](ADR-016-onboard-audio-no-pcm3060.md)）。
- Seed pin 16–19 仍为 **AC 耦合** line level（±1.8V 量级）。
- 这意味着 `IN_L/R` 适合看音频与波形形状，不适合判断慢速/静态 CV 的绝对偏置电压。
- 在 Scope 中：
  - `A IN (IN_L/R)`：可用于频率、相位、形状观察；
  - `CV ADC (CV1–CV4)`：用于绝对电平、单极/双极偏置判断。
- 若需在音频口上做绝对电压测量，需硬件级改动（DC 耦合前端），不能仅靠 UI 或 App 参数切换完成。
