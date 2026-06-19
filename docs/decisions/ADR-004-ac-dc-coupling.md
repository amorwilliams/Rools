# ADR-004: AC/DC 耦合策略

## 状态

已接受

## 决策

| 接口 | 硬件 | 软件 |
|------|------|------|
| IN_L/R, OUT_L/R | DC 耦合 ±10V | AC/DC 可选（IIR 高通 ~1–20 Hz） |
| CV1–CV4, CV A–D | DC 耦合 | 固定 DC |

CV 输出必须 DC（1V/oct、包络等）。音频 FX 默认软件 AC；示波/频谱默认 DC。

## 理由

- 8HP/10HP 无空间做 per-jack 物理 AC/DC 开关
- O'Tools 类测量需 DC 输入
- 软件高通满足 FX 去 DC 偏置

## 后果

- `CouplingMode` 在 AudioEngine 与 App 默认值中实现
- 不做面板耦合开关

## 实现提醒（Seed/Patch 参考链路）

- Daisy Seed 数据手册说明 `Audio inputs are AC coupled`（音频输入为 AC 耦合）。
- 这意味着 `IN_L/R` 适合看音频与波形形状，不适合判断慢速/静态 CV 的绝对偏置电压。
- 在 Scope 中：
  - `A IN (IN_L/R)`：可用于频率、相位、形状观察；
  - `CV ADC (CV1–CV4)`：用于绝对电平、单极/双极偏置判断。
- 若需在音频口上做绝对电压测量，需硬件级改动（DC 耦合前端），不能仅靠 UI 或 App 参数切换完成。
