# ADR-006: 外置 4 通道 CV DAC

## 状态

已接受

## 背景

面板 CV Out A–D 共 4 路；Daisy Seed 仅 2 路 12-bit 内置 DAC。

## 决策

v1 采用 **MCP4728**（I2C，4× 12-bit），经运放至 ±10V。Seed 内置 DAC 闲置。

备选：DAC8554（SPI，16-bit）— 若 v2 需更高 CV 精度再评估。

## 理由

- 单芯片 4 路，BOM 低
- 与 Codec 共用 I2C，不占 SPI（SPI 已给显示屏）
- 12-bit 够 LFO/Trig/包络；1V/oct 精度有限但 v1 非主用途

## 后果

- PCB 增加 I2C 器件与 4 路运放
- 需 CV Out 校准流程
