# ADR-015: 设置持久化与 Flash 磨损策略

## 状态

已接受

## 背景

Rools 需在 QSPI Flash 中持久化：

- CV 逐通道校准 (`GlobalSettings.cv[4]`)
- 上次打开的 App (`last_app_index`)
- 未来各 App / 菜单参数

Flash 物理特性：**最小擦除粒度 4KB**。无论修改 1 字节还是整扇区，一次 `Save` 消耗 **1 次扇区擦写寿命**（约 10 万次/扇区，IS25LP080D）。

Daisy Seed 面包板阶段**无**掉电保持电容，拔电瞬间无法可靠落盘；M2 PCB 已画比较器 + hold-up 前端（见 §3）。

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

### 3. 掉电检测保存（M2 PCB + 固件 debounce）

Power 页（Rev 0.2）已画掉电前端，Seed **D28 / pin 35** 接 net **`EXTI_PWR`**：

| 硬件 | 件 / 参数 | 作用 |
|------|-----------|------|
| 比较器 | **U10A LM393** | +12V 分压（R43 22k / R64 10k）vs `+3V3_D` 基准；开漏输出 + R65 10k 上拉 |
| 触发阈值 | ~**10.6 V** | \(V_{trip} = 3.3 \times 32/10\)；正常 +12V 时分压 ≈ 3.75 V，`EXTI_PWR` 高 |
| 保持 | **D3 SS34** → `+12V_SAFE` + **C36/C37 各 1000 µF** | 共 ~2000 µF，维持 Buck/比较器/写 Flash 窗口 |
| MCU 输入 | D28（`EXTI_PWR`） | 持续 LOW 表示掉电预警 |

**软件**（`firmware/src/board/pwr_fail.{h,cpp}`）：

- **1 kHz 定时器 ISR** 读 D28；连续 LOW ≥ **5 ms** 才认定掉电（过滤毛刺；面包板 D28 未接时不触发）
- 确认掉电后顺序：**`EmergencyBacklightOff()`**（GPIO 关背光，~30–50 mA）→ **`SettingsStore::OnPowerFail()`** → `Flush()`
- 禁止 SPI 刷屏 / DMA / audio 阻塞
- `HasPowerFailHardware()` → `true`（无编译宏；面包板靠 debounce 不误触）

M2 启用后可切换 **PowerFailOnly** 策略：运行期仅改 RAM，每次开关机 1 次擦写，与环形缓冲可叠加。

### 4. 演进路径（非本期）

1. 环形缓冲（本期）
2. 掉电 debounce 保存（M2 PCB + hold-up，见 [ADR-011](ADR-011-power-supply.md)）
3. 可选外挂 FRAM（高频序列器等）

## M2 掉电保存 hardware / 软件 checklist

- [x] 掉电前端原理图（LM393 + hold-up，Power 页 Rev 0.2）
- [ ] 保持电容容量：\(C \geq I_{hold} \cdot \Delta t / \Delta V\)（hold 电流、目标维持 ms、允许压降）
- [ ] 比较器阈值实测：+12V 跌至 ~10.6 V 前触发，Buck 仍可供电
- [x] D28 / `EXTI_PWR` 接线（Seed 页）；勿 D13（Exp MIDI TX）
- [x] 固件：1 kHz debounce + 关背光 + `Flush()`
- [ ] 与环形缓冲联调：掉电写下一槽，sequence 递增

## 后果

- 原型/DIY：环缓 + 节流足够（每天 100 次保存 ≈ 27 年量级）
- 面包板拔电可能丢失未 Flush 的 `last_app_index` 等（2s 窗口内）；无 hold-up 时掉电保存不可用
- 校准 App 已显式 `Flush()`

## 相关

- [ADR-011](ADR-011-power-supply.md) — 电源架构与 hold-up
- [05-software-architecture.md](../05-software-architecture.md) — AppShell / Settings 集成
