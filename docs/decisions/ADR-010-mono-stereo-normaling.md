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

使用 **3.5 mm 立体声插座带 normalize 簧片**（如 Thonkiconn PJ398SM、PJ301M-12 等，以 KiCad 库为准）：

```
IN_L Tip  ──► Codec Left In
IN_R Tip  ──► Codec Right In
IN_L Tip  ──► IN_R Switch（normalize）
```

- **IN_R 未插线**：Switch 闭合，IN_R Tip = IN_L 信号 → Codec 右声道 = 左声道
- **IN_R 插入**：Switch 断开，IN_R 仅接收自己的 Tip
- **无需软件检测**；`MonoMode::Auto` 信任此硬件行为

Sleeve / Ring：按 Eurorack mono 惯例接地；具体以 Codec 前端单端/差分方案为准。

## 待验证

- [ ] KiCad 网表：IN_R Switch 脚连 IN_L Tip
- [ ] 示波器：空插 R 时 R 通道波形 = L
- [ ] 插入 R 后 R 独立
