# ADR-007: USB-A Host 存储

## 状态

已接受

## 背景

Granular 等 App 需样本库；预设超出 Seed 8MB Flash；用户希望现场可编辑。

## 决策

Exp 面板 **USB-A Host**（非 USB-C），仿 [arbhar](https://www.instruomodular.com/wp-content/uploads/2023/12/arbhar-V2-Sample-Managment.pdf) / [Lúbadh](https://www.instruomodular.com/wp-content/uploads/2022/11/Lubadh-Manual-Firmare-V2-A5-.pdf)：

```
/rools/samples/
/rools/presets/
/rools/scenes/
```

首次插入自动初始化。WAV 48kHz/24bit 优先；预设 JSON。

## 理由

- USB-A 母座插 U 盘更牢靠，符合模块现场习惯
- O_C 4.1 同思路（USB HOST）
- Seed Micro USB 保留开发用，不占面板

## 后果

- +5V Host 供电与限流（~500 mA）
- 固件 FAT32 + USB Host 栈
- 无 Exp 时 Granular 等功能受限
