# ADR-006: 外置 4 通道 CV DAC

## 状态

已接受（2026-06 修订：MCP4728 → DAC8565）

## 背景

面板 CV Out A–D 共 4 路；Daisy Seed 仅 2 路 12-bit 内置 DAC。

## 决策

v1 采用 **DAC8565**（SPI，4× 16-bit）；**AVDD=`+3V3_DAC`**、**IOVDD=`+3V3_D`**（内部 2.5V 基准，VREFL→GND）；经 **OPA4171**（R=12.5k/100k，增益 −8）调理至 Eurorack ±10V（DAC 0–2.5V → jack +10V～−10V）。Seed 内置 DAC 闲置。

| 信号 | Seed GPIO | Seed pin | 网表 |
|------|-----------|----------|------|
| SPI1_SCK | D8 | 10 | 与 ST7735 共用 |
| SPI1_MOSI | D10 | 11 | 与 ST7735 共用 |
| DAC_CS (SYNC) | D26 | 33 | `DAC_CS` |
| DAC_LDAC | D27 | 34 | `DAC_LDAC` |

D11–D12 → Exp I2C；D13–D14 → Exp MIDI（USART1）。

## 理由

- 16-bit 优于 12-bit MCP4728，利于 CV 精度
- 与 LCD 共用 SPI1，独立 CS/LDAC，不占 I2C
- OPA4171 低功耗四运放，单芯片覆盖 4 路输出调理

## 后果

- 固件 `CvOutDriver` SPI 写 + LDAC 脉冲同步四路
- 需 CV Out 校准流程

## 待验证

- [ ] 四路 0V / ±10V 实测（M2）
- [ ] LDAC 批量更新无通道错位
