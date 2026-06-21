# ADR-014: 4 层 PCB 叠层与分区

## 状态

已接受

## 背景

Rools Core（+ 可选 Exp）单块或叠板 PCB 同时包含：

- **数字**：Daisy Seed（STM32H7）、ST7735 SPI、DAC8565 SPI、USB OTG Host、MIDI UART
- **模拟**：Eurorack ±10V 音频 In/Out、4× CV In/Out + 运放调理
- **电源**：[ADR-011](ADR-011-power-supply.md) Buck 开关降压（开关噪声、大电流回流）

2 层板无完整地平面，数字/模拟/电源难以分区；Buck 与 Codec/CV 邻域时易出现 hum、CV 噪声、USB 不稳定。同类模块（Daisy Patch、O_C、MFX）量产多采用 **4 层**。

10HP（~50 mm）布线密度高：12× 3.5 mm jack + 屏 + Seed + 电源，2 层需大量跳线式地线，layout 与调试成本高。

## 决策

1. **M2 正式 PCB 打样采用 4 层**，标准厚度 **1.6 mm**（不增加 [ADR-013](ADR-013-mechanical-depth.md) 深度预算）。
2. **不推荐 2 层作为量产方案**；仅当单块极低成本 Core-only 原型且接受更长调试周期时可例外。
3. **推荐叠层**（JLC / 常见 4L1.6 mm 工艺）：

| 层 | 用途 |
|----|------|
| L1（Top） | 元件面：Seed、Buck、屏接口、数字连接器；模拟 jack 靠板边 |
| L2 | **完整地平面 GND**（尽量不分割） |
| L3 | **+5V / +3.3V 铺铜** + 少量信号；Buck 输入/输出短路径 |
| L4（Bottom） | 信号、Eurorack 16-pin 电源、部分插孔 |

4. **分区原则**：
   - Buck、Seed、屏、USB：**数字区**；开关结与电感远离 Codec/CV 运放
   - Codec、CV In/Out 运放：**模拟区**；模拟地 AGND 于 L2 单点汇 DGND（星形/桥接，KiCad 定网表）
   - USB 差分对：L1 走线，**L2 完整地参考**，目标 ~90 Ω
   - I2S / SPI：短、直；不跨 analog 区上方长距离飞线

5. Exp 与 Core **同 4 层板** 或 FFC 连接子板；子板同样优先 4 层（含 USB Host）。

## 理由

- 连续 GND 平面是 Eurorack 音频/CV 底噪的基础
- Buck + USB 500 mA 需要低阻抗电源与回流路径
- 4 层小批量加价通常可接受（相对调试与返工成本）
- 1.6 mm 四板与双板等厚，不影响机箱深度

## 后果

- KiCad 项目设 4-layer stackup；fab 备注指定阻抗（USB）若代工厂支持
- BOM 中 PCB 成本略高于 2 层（见 [bom-draft.md](../../hardware/bom/bom-draft.md)）
- Layout 需明确 AGND/DGND 连接点（一处，靠近 Codec）

## 待验证

- [ ] 首版 PCB：空载 / 满载音频 THD+噪声与 CV 噪声
- [ ] USB U 盘连续读写 + 频谱 UI 同时运行
- [ ] Buck 开关频率谐波在音频带内是否可接受（必要时调整 L/C 或频率）
