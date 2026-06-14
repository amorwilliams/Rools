# ADR-013: 模块深度与机械布局

## 状态

已接受

## 背景

目标深度 **35–40 mm**（对齐 O_C 4.1）。主要堆叠：

- 前面板（2 mm Alu）+ 控件轴长
- 1.77" TFT + FPC/排针
- Daisy Seed（排母插接很占 Z 高度）
- Eurorack 插孔（Thonkiconn 筒深）
- Exp：**USB-A 母座** 为最大深度约束之一

## 决策

KiCad 阶段在以下方案中 **选最薄可行组合**（可混合）：

| 约束 | 优先方案 | 备选 |
|------|----------|------|
| Daisy Seed | **贴片焊接** 于主 PCB（最薄） | 1.27 mm 矮排针 + 无排母直插 |
| Seed 可维护性 | 若必须可拆：矮排母 + 侧面留测试点 | 牺牲 3–5 mm |
| USB-A（Exp） | **沉板式** 母座 或 **垂直子板**（Exp PCB 90° 相对 Core） | 标准竖插母座（最深，尽量避免） |
| 屏 | 排针/FPC 贴 PCB，背光常亮或 GPIO 控 | — |
| Pot / Enc | 选 **薄型** 轴（9 mm pot 已选） | — |

**深度预算**：Layout 前建 Excel/mm 叠层表，合计 ≤ 38 mm（留 2 mm 公差）。

## 理由

- 35 mm 内排母 + 标准 USB-A + 标准 Seed 叠放几乎不可能
- O_C 4.1 同档深度是产品对标；超标则难进浅箱

## 后果

- 贴片 Seed 降低现场换 MCU 便利性；调试靠 SWD + USB Micro
- Exp 垂直子板可能影响面板 USB 开口机械；Gerber 前 3D 装配验证
- `04-hardware-architecture.md` 深度节引用本 ADR

## 待验证

- [ ] KiCad 3D：Core / Core+Exp 总深度
- [ ] 最短 Eurorack 电源线 + 模块同时插入机箱
- [ ] 面板开孔与 USB-A 对齐（Exp）
