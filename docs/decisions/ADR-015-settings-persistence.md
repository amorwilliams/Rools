# ADR-015: 设置持久化与 Flash 磨损策略

## 状态

已接受

## 背景

Rools 需在 QSPI Flash 中持久化：

- CV 逐通道校准 (`GlobalSettings.cv[4]`)
- 上次打开的 App (`last_app_index`)
- 未来各 App / 菜单参数

Flash 物理特性：**最小擦除粒度 4KB**。无论修改 1 字节还是整扇区，一次 `Save` 消耗 **1 次扇区擦写寿命**（约 10 万次/扇区，IS25LP080D）。

Daisy Seed 面包板阶段**无**掉电保持电容与比较器，无法在拔电瞬间可靠落盘。

## 决策

### 1. 统一 `GlobalSettings` 聚合 + `SettingsStore`

- 单块 RAM 工作副本；`MarkDirty()` + 2s 节流 `Tick()` 自动落盘
- 切 App、CV 校准完成时 `Flush()` 立即落盘
- 实现见 `firmware/src/settings/settings_store.{h,cpp}`

### 2. QSPI 环形缓冲区磨损均衡（本期）

- **10 槽 × 4KB** 轮转写入，sequence 单调递增，开机取最大有效槽
- 理论寿命 ≈ **10 万次 × 10 = 100 万次**擦写
- 无变化时跳过写入（`settings == persisted`）
- 一次性迁移旧 `libDaisy PersistentStorage` 单槽格式

### 3. 掉电检测保存（M2 PCB，本期仅预留接口）

面包板**不实现**。量产 PCB 计划：

| 硬件 | 作用 |
|------|------|
| +5V 保持电容 | 拔电后维持数十 ms 供 MCU 写 Flash |
| 比较器监测 +12V 或 +5V 跌落 | 提前触发，不等电压归零 |
| 比较器 → Seed GPIO EXTI | 下降沿最高优先级 ISR |

软件预留（本期空实现）：

- `SettingsStore::HasPowerFailHardware()` → `false`
- `SettingsStore::InitPowerFailDetection()` → no-op
- `SettingsStore::OnPowerFail()` → `Flush()`（有变化才写）
- `pins.h` 预留 `kPwrFail` 引脚注释

M2 启用后可切换 **PowerFailOnly** 策略：运行期仅改 RAM，每次开关机 1 次擦写，与环形缓冲可叠加。

### 4. 演进路径（非本期）

1. 环形缓冲（本期）
2. 掉电 EXTI 保存（M2 PCB + 保持电容，见 [ADR-011](ADR-011-power-supply.md) 电源段扩展）
3. 可选外挂 FRAM（高频序列器等）

## M2 掉电保存硬件 checklist

- [ ] 保持电容容量：\(C \geq I_{hold} \cdot \Delta t / \Delta V\)（hold 电流、目标维持 ms、允许压降）
- [ ] 比较器阈值：在 Buck 仍能给 Seed 供电前触发（勿等 3.3V 掉光）
- [ ] EXTI 引脚：空闲 GPIO（如 D13），下降沿，上拉
- [ ] ISR 内仅 `OnPowerFail()` → `Flush()`；禁止 SPI 显示、禁止 audio 阻塞
- [ ] 与环形缓冲联调：掉电写下一槽，sequence 递增

## 后果

- 原型/DIY：环缓 + 节流足够（每天 100 次保存 ≈ 27 年量级）
- 面包板拔电可能丢失未 Flush 的 `last_app_index` 等（2s 窗口内）
- M2 前勿依赖掉电保存；校准 App 已显式 `Flush()`

## 相关

- [ADR-011](ADR-011-power-supply.md) — 电源架构
- [05-software-architecture.md](../05-software-architecture.md) — AppShell / Settings 集成
