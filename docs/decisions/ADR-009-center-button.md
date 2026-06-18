# ADR-009: 双 Encoder 中间单键

## 状态

已接受

## 背景

曾考虑 O_C 式 A/B/X/Y 四小键；10HP + 1.77" 屏侧无余量。

## 决策

- **Enc A + Push**、**Enc B + Push**（仿 O_C 双 Encoder）
- **1× 6 mm tact** 置于两 Encoder 中间
- 取消 A/B/X/Y 四键

Btn 默认：Shift / 全屏 / 确认。

### 交互约定（补充）

- **Btn 短按**：App 级二级动作（如 Fullscreen / Hold / Toggle）。
- **Btn 长按（>220 ms）**：进入全局 `Shift` 按住态；松开即退出。
- **Shift + Enc A/B**：触发二级参数层或快捷功能（由 App 定义）。
- 非 Shift 时保持主路径一致：`Enc A` 导航、`Enc B` 参数、`Btn` 执行次动作。

### 优先级与冲突规则（固定）

- 菜单打开时，仅菜单消费输入，不透传 App。
- 菜单关闭时，`Shift + Enc` 优先于普通 `Enc`。
- 长按触发 Shift 后，释放 Btn 不再产生短按事件。
- 避免同帧双模式：`Enc A` 长按菜单与 Btn 长按 Shift 不叠加触发。

## 理由

- O_C 两 Encoder 中间单键已验证 UX
- 比四键省 3 GPIO，10HP 横向更宽松

## 后果

- App 快捷键少于 O_C，依赖 Enc 按压与 Btn 组合
