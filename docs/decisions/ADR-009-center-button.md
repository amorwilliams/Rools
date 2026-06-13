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

## 理由

- O_C 两 Encoder 中间单键已验证 UX
- 比四键省 3 GPIO，10HP 横向更宽松

## 后果

- App 快捷键少于 O_C，依赖 Enc 按压与 Btn 组合
