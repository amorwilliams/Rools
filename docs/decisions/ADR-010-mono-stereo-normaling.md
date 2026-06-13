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

- PCB 需 switching/normalled jack 或等效电路
- KiCad 阶段确认 Thonkiconn normalize 脚接法
