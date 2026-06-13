# BOM 草案（M0）

> 非最终采购清单；封装/供应商 KiCad 阶段再定。

## 核心数字

|  Ref | 件 | 数量 | 备注 |
|------|-----|------|------|
| U1 | Daisy Seed | 1 | 64MB SDRAM 版 |
| U2 | PCM3060 或 CS4272 等 | 1 | Eurorack 音频 Codec |
| U3 | MCP4728 | 1 | 4ch I2C DAC |
| U4 | ST7735 1.77" IPS | 1 | 160×128，SPI |
| — | STM32 USB Host PHY / 5V switch | 1 | 视 Seed OTG 设计 |

## 模拟 / 电源

|  Ref | 件 | 数量 | 备注 |
|------|-----|------|------|
| — | Eurorack 电源入口 | 1 | 16-pin 2×5 shrouded |
| — | ±12V → 5V/3.3V DC-DC | 1 | |
| — | 运放（CV in/out 调理） | 若干 | OPA1612 等 |
| — | 保护/TVS | 若干 | CV/Audio 输入 |

## 控件

|  Ref | 件 | 数量 | 备注 |
|------|-----|------|------|
| — | Rotary Encoder w/ switch | 2 | |
| — | Tact 6×6 mm | 1 | 中间 Btn |
| — | 9 mm pot | 4 | K1–K4 |
| — | Knob 帽 | 6 | 2 Enc + 4 Pot |

## 连接器

|  Ref | 件 | 数量 | 备注 |
|------|-----|------|------|
| J1–J12 | 3.5 mm mono jack | 12 | Thonkiconn；IN_R normalled |
| — | MIDI jack | 2 | Exp |
| — | USB-A 母座 | 1 | Exp |

## 面板 / 机械

|  Ref | 件 | 数量 | 备注 |
|------|-----|------|------|
| — | 10HP 面板 | 1 | 2 mm Alu |
| — | 2HP Exp 面板 | 1 | 可选 |
| — | M3 螺丝/柱 | 套 | |

## 预估 BOM 成本（粗略）

| 类别 | USD（估） |
|------|----------|
| Daisy Seed | ~30 |
| 屏 + Codec + DAC | ~25–40 |
| 被动/运放/电源 | ~15–25 |
| 控件+孔 | ~20–30 |
| PCB+面板 | ~30–50 |
| **合计** | **~120–175**（单件原型） |
