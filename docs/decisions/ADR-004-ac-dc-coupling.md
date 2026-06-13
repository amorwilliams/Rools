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
