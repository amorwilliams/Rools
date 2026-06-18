# 06 — UI/UX 交互规范

## 控件默认行为

| 控件 | 默认 | App 可覆盖 |
|------|------|-----------|
| Enc A 旋转 | App 菜单 / 列表滚动 | 是 |
| Enc A 按压 | 确认 / 进入 | 是 |
| Enc B 旋转 | 参数微调 | 是 |
| Enc B 按压 | 返回 | 是 |
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
