# ADR-010: Mono / Stereo 与 Normalled

## 状态

已接受

## 背景

用户期望只插 IN_L 时 Mono，输出 L+R 均有声。

## 决策

| 环节 | 实现 |
|------|------|
| 输入 Mono | **硬件**：IN_R jack **normalled 到 IN_L** |
| 输出 L→R | **软件**：`AudioEngine` 中 `out_R = out_L`（Mono 模式） |
| 插线检测 | **不做**纯软件检测（标准 jack 无 sense） |

菜单：`Input: Auto / Mono / Stereo`；`Output: Stereo / Mono(L→R)`。

## 理由

- Eurorack 惯例用 normalled 解决 Mono in
- 输出复制必须 DSP，用户常要双路输出同一信号

## 后果

- PCB 需 **带 Switch 触点的 normalled jack**（见下）
- KiCad 阶段核对 footprint 与网表

## 硬件接法（Thonkiconn / PJ398SM）

KiCad 网 **`IN_L_NORM`**：

```
J_CV_5 (IN_L) Tip ──► TL074 / Seed Left In
J_CV_6 (IN_R) Tip ──► TL074 / Seed Right In
J_CV_5 (IN_L) Tip ──► J_CV_6 (IN_R) Switch  （net: IN_L_NORM）
```

- **IN_R 未插线**：Switch 闭合，IN_R 收到 IN_L 信号 → 右声道 = 左声道
- **IN_R 插入**：Switch 断开，IN_R 仅接收自己的 Tip
- **无需软件检测**；`MonoMode::Auto` 信任此硬件行为

Sleeve：按 Eurorack mono 惯例接地。

## 待验证

- [x] KiCad 网表：`IN_L_NORM`（J_CV_5 Tip ↔ J_CV_6 Switch）
- [ ] 示波器：空插 R 时 R 通道波形 = L
- [ ] 插入 R 后 R 独立
