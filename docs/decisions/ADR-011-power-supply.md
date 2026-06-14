# ADR-011: 内部电源与 USB Host 供电

## 状态

已接受

## 背景

Rools Core + Exp 负载包括：

| 子系统 | 典型 +5V 电流（估） |
|--------|-------------------|
| Daisy Seed | ~80–120 mA |
| ST7735 背光 | ~30–50 mA |
| 外置 Codec + 运放 | ~30–60 mA |
| MCP4728 + CV 调理 | ~10–20 mA |
| USB-A Host（U 盘） | ~50–100 mA 典型；规范上限 **500 mA** |

Eurorack 机箱 **+5V 轨往往很弱或不存在**；模块必须从 **+12V（及必要时 -12V）** 自产 +5V / +3.3V。

线性稳压（7805、LM1117 等）从 +12V 降压到 +5V：压差 ~7V × 300mA ≈ **2W+ 发热**，10HP 模块会烫手且不稳定。

## 决策

1. **+5V / +3.3V 一律用开关降压（Buck）**，禁止 7805 / LM1117 作为主 +5V 路径。
2. **不从 Eurorack 面板 +5V 取电**（若机箱有也仅作备用/debug，不纳入设计）。
3. **+12V 入口**：DC-DC Buck → +5V 母线 → Seed / 屏 / Codec 数字 / USB VBUS 开关。
4. **USB Host**：独立 **限流开关**（如 TPS20xx / AP2553 类，500 mA 可配置），带故障指示；U 盘未插时不向 VBUS 供电。
5. KiCad 阶段做 **电流预算表** 并留 **≥30% 裕量**；Exp 未装时 Core 单独测功耗。

### 参考器件（非最终 BOM）

| 功能 | 方向 |
|------|------|
| +12V → +5V | Buck，≥1 A 连续（如 TPS54331、MP1584、同类） |
| +5V → +3.3V | LDO 可接受（压差小、电流低） |
| USB VBUS | 限流 High-side switch + ESD |

## 理由

- 与 Daisy Patch / 商用 Eurorack 数字模块惯例一致
- USB Host 500 mA 峰值必须在 PCB 级可控，不能靠 Seed 裸供电
- 热设计在 35 mm 深度内无散热片空间

## 后果

- BOM 成本略高于线性方案；布局需 Buck 电感 + 足够铜皮
- `01-product-spec` 电流预估值须 M2 实测修订
- 见 [hardware/bom/bom-draft.md](../../hardware/bom/bom-draft.md) 电源行

## 待验证

- [ ] Core-only / Core+Exp 满载 +12V 电流（示波器 + 万用表）
- [ ] U 盘插入浪涌与 Buck 跌落
- [ ] 连续运行 30 min 外壳温升
