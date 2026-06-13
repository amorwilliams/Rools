# Rools Exp — 2HP 面板规格

## 外形

| 项目 | 值 |
|------|-----|
| 宽度 | 2 HP = 10.16 mm（实际 ~10.0 mm） |
| 高度 | 128.5 mm |
| 厚度 | 2 mm 铝 |

## 开孔（自上而下）

| 序号 | 件 | 说明 |
|------|-----|------|
| 1 | MIDI IN | 3.5 mm TRS 或 2× DIN（待定） |
| 2 | MIDI OUT | 同左 |
| 3 | USB-A 母座 | 垂直或 90° 出板；注意深度 |

## 丝印

- MIDI IN / MIDI OUT
- USB HOST

## 机械

- 与 Core PCB 同深度目标 35 mm
- USB-A 全尺寸为深度关键件；可选 90° 母座减深度
- Exp 与 Core 面板相邻安装，共享或 FFC 连接 Core PCB

## 电气连接

- Exp 不独立供电：排针/FFC 从 Core 取 5V Host 供电、MIDI UART、USB D+/D-

## 待定

- [ ] MIDI TRS Type A vs B 标注
- [ ] USB 母座型号（深度 vs 牢靠）
