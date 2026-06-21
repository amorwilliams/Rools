# BOM 草案（M0）

> 非最终采购清单；封装/供应商 KiCad 阶段再定。

## 核心数字

|  Ref | 件 | 数量 | 备注 |
|------|-----|------|------|
| U1 | Daisy Seed | 1 | 64MB SDRAM 版 |
| U9 | DAC8565 | 1 | 4ch SPI DAC；内部 2.5V 基准；AVDD=`+3V3_DAC`，IOVDD=`+3V3_D` |
| U3 | ST7735 1.77" IPS | 1 | 160×128，SPI |
| — | STM32 USB Host PHY / 5V switch | 1 | 视 Seed OTG 设计 |

## 模拟 / 电源

|  Ref | 件 | 数量 | 备注 |
|------|-----|------|------|
| — | Eurorack 电源入口 | 1 | 16-pin 2×5 shrouded |
| — | **+12V → +5V Buck** | 1 | **TPS54302**（U2）；≥1 A；禁 7805/1117 主路径（[ADR-011](../../docs/decisions/ADR-011-power-supply.md)） |
| — | +5V → +3.3V LDO | 1 | **AP2112K-3.3** → `+3V3_D` |
| U10 | **LM393** 双比较器 | 1 | U10A 掉电检测 → `EXTI_PWR` / D28 |
| — | 掉电 hold-up | 2 | **1000 µF/25V**（C36/C37）+ **SS34**（D3）→ `+12V_SAFE` |
| — | USB VBUS 限流开关 | 1 | ~500 mA；Exp Host（[ADR-011](../../docs/decisions/ADR-011-power-supply.md)） |
| — | 运放 | 若干 | **TL074**（音频 ±10V）；**MCP6004**（CV In）；**OPA4171**（CV Out） |
| — | 精密电阻 0.1% | 12 | CV Out：**12.5k ×4**（R7/R9/R11/R13）+ **100k ×8**（R8/R10/R12/R14/R15–R18） |
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
| J1–J12 | 3.5 mm mono jack | 12 | Thonkiconn **PJ398SM** 等；IN_R Switch→IN_L Tip（[ADR-010](../../docs/decisions/ADR-010-mono-stereo-normaling.md)） |
| — | MIDI jack | 2 | Exp |
| — | USB-A 母座 | 1 | Exp；**沉板或垂直子板**（[ADR-013](../../docs/decisions/ADR-013-mechanical-depth.md)） |

## 面板 / 机械

|  Ref | 件 | 数量 | 备注 |
|------|-----|------|------|
| — | **PCB 4 层 1.6 mm** | 1 | [ADR-014](../../docs/decisions/ADR-014-pcb-4-layer.md)；JLC 等 |
| — | 10HP 面板 | 1 | 2 mm Alu |
| — | 2HP Exp 面板 | 1 | 可选 |
| — | M3 螺丝/柱 | 套 | |

## 预估 BOM 成本（粗略）

| 类别 | USD（估） |
|------|----------|
| Daisy Seed | ~30 |
| 屏 + DAC8565 + OPA4171 | ~15–25 |
| 被动/运放/电源 | ~15–25 |
| 控件+孔 | ~20–30 |
| PCB+面板 | ~30–50 |
| **合计** | **~120–175**（单件原型） |
