# 06 — UI/UX 交互规范

## 控件默认行为

| 控件 | 默认 | App 可覆盖 |
|------|------|-----------|
| Enc A 旋转 | 选择当前参数 | 是 |
| Enc A 按压 | 确认 / 进入 | 是 |
| Enc B 旋转 | 修改当前参数值 | 是 |
| Enc B 按压 | 切换 Coarse / Fine | 是 |
| Btn（中）短按 | App 二级动作（Fullscreen/Hold/Toggle） | 是 |
| Btn（中）长按 | Shift（按住有效，松开退出） | 否（全局） |
| K1–K4 | 当前 App 第 1–4 列参数 | 语义随 App |
| CV1–CV4 | 叠加到 K1–K4 | 始终 |
| CV A–D | App 分配输出 | App 定义 |

## Knob ↔ CV 默认语义

| 列 | Knob | CV In | 典型 App 映射 |
|----|------|-------|--------------|
| 1 | K1 | CV1 | 主参数 A（Time） |
| 2 | K2 | CV2 | 主参数 B（Feedback） |
| 3 | K3 | CV3 | Clock / Mod |
| 4 | K4 | CV4 | Trig / Mod |

App 切换时**物理列不变**，仅语义变。

## 显示模式

| 模式 | 内容 |
|------|------|
| Normal | 参数 + 小波形/频谱 |
| Fullscreen | 全屏波形 / 频谱 / 李萨如（Btn 或 Enc B 切换） |

## UI 布局架构（LayoutView）

### 分层职责

- `AppShell`：只负责输入时序、菜单状态路由、调用布局入口，不直接画 Top/Main/Bottom。
- `LayoutView`：统一控制 Top/Main/Bottom 绘制与分区刷新。
- `App`：只负责 Main 区内容绘制，通过 `ui_draw(const LayoutMetrics&)` 接收边界。

### 布局数据

`LayoutMetrics` 统一提供以下字段：

- `top_height`
- `bottom_height`
- `main_top`
- `main_bottom`
- `main_height`

### 渲染主流程

App 状态下统一走：

1. `DrawTopIfDirty(app)`
2. `app->ui_draw(layout)`
3. `DrawBottomIfDirty(app, shift_active)`
4. 分区 `FlushRect`（Top/Main/Bottom）

入口函数：`LayoutView::RenderAppFrame(App* app, bool shift_active)`。

### 菜单态约束

- 菜单打开时由菜单视图独占渲染。
- 进菜单/出菜单后执行 `LayoutView::ResetCache()`，确保回到 App 时 Top/Bottom 不残留脏缓存。

## Scope 交互（固定顺序4槽）

### 固定顺序路由策略

- 后台常驻采样 6 路输入：`Audio1`、`Audio2`、`CV1`、`CV2`、`CV3`、`CV4`。
- 前台固定 4 个显示槽位，不做补位重排。
- 路由优先级支持 `AudioFirst` / `CvFirst` 两种模式，且顺序固定：
  - `AudioFirst`：`T1=AIN1`、`T2=AIN2`、`T3=CV1`、`T4=CV2`
  - `CvFirst`：`T1=CV1`、`T2=CV2`、`T3=CV3`、`T4=CV4`
- 激活判定采用“GPIO优先 + ADC兜底”：
  - 有插座检测 GPIO 时，GPIO 作为真值来源；
  - 无 GPIO 时，使用短窗 `RMS/Peak` 阈值判定输入是否活跃。
- 路由切换带短保持（hysteresis + hold），降低插拔和噪声导致的跳轨。
- 任一槽位无信号时，槽位标签仍显示绑定接口名，仅通过颜色表达无信号状态。

### 单触发源规则

- 触发搜索只对 1 路执行（默认第一个可见槽位）。
- 其余已显示轨道共享同一个 `start_linear/subsample` 起点绘制。
- `AUTO`：无触发时自由运行；`NORM`：无触发时不绘制波形。

### 轨道选择与调参

- `Enc A` 旋转：切换焦点参数。
- `Enc B` 旋转：调整焦点参数值（作用于当前选中轨道）。
- `Enc A` 按下：选中上一条可见轨道。
- `Enc B` 按下：选中下一条可见轨道。
- `Shift + Enc B` 按下：切换 `Coarse/Fine` 步进。
- `Btn` 短按：`Hold/Unhold`。
- `Shift + Enc B` 旋转：`Hold/Unhold` 快捷切换。

### 渲染要求

- 4 轨同屏时自动走 Fast 路径（关闭 AA）以稳定帧率。
- 选中轨道高亮，非选中轨道降亮度。
- 底栏显示参数值（`B:<参数值>`，保留单位）。
- 无可见输入时主区域不显示文本提示（仅保留网格与状态栏信息）。

## Spectrum 高级交互

- 新增分析参数：`PeakTrack`、`Cursor`、`Freeze`（并保留 `Gain`/`Decay`/`FFT`/`PeakHold`）。
- `Enc A`：切换当前参数焦点；`Enc B`：调整当前焦点值。
- `Btn` 短按：`Freeze/Unfreeze`（冻结仅冻结分析画面，不中断音频直通）。
- `Shift + Enc B`：全屏切换。
- 视图覆盖层：
  - PeakTrack 开启时显示峰值频点标记线。
  - Cursor 开启时显示游标竖线；状态行显示 Peak/Cursor 的 `Hz + 归一化幅值`。
  - Freeze 开启时显示 `FREEZE` 标记。

## App 切换流程

1. Enc A 长按（>500 ms）→ App 菜单
2. Enc A 旋转选择
3. Enc A 短按确认
4. 交叉淡入淡出 50 ms → 新 App

## 菜单层级

```
[Fullscreen View] ← Btn
       ↑
[App Main UI] ← Enc B 返回
       ↑
[App Menu] ← Enc A 长按
       ↑
[Global Settings] ← 从 App Menu 进入（耦合、Mono、USB…）
```

## Shift 约定

- 长按 Btn（>220 ms）进入 Shift，松开退出。
- Shift 期间：`Enc A/B` 进入二级参数或快捷动作。
- 非 Shift 期间：保持主流程（A 导航、B 参数、Btn 次动作）。

## 手势优先级与冲突规则

- 菜单态优先：App 菜单打开时，手势仅供菜单消费，不透传到 App。
- Shift 次之：菜单关闭时，`Shift + Enc` 优先于普通 `Enc` 旋转。
- `CenterTap` 仅在短按时触发；长按进入 Shift 后，释放不会再触发短按。
- `Enc A` 长按开菜单与 `Btn` 长按进 Shift 相互独立，避免同帧双触发。

## 反馈

| 事件 | 反馈 |
|------|------|
| 参数变更 | 屏上数值更新；可选短促 UI 音（v2） |
| USB 刷固件 | 全屏「Updating…」+ 进度 |
| 错误 | 红色文字 + update.log |

## 无障碍 / 现场

- K1–K4 与 CV 列对齐，盲操可依赖列位置
- Encoder 负责精确数值，Knob 负责宏观扫参
- 全屏模式一键（Btn）便于远处查看频谱
