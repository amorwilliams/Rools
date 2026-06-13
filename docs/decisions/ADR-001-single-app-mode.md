# ADR-001: 单 App 运行模式

## 状态

已接受

## 背景

Rools 定位为多功能平台，需在 App 并行（如 O_C Quadrants）与单 App 之间选择。

## 决策

**一次只运行一个 App**。切换需退出当前 App（`on_exit` → `on_enter`），50 ms 音频 crossfade。

## 理由

- 初学者开发与调试更简单
- 10HP UI 不足以支撑多 App 并行状态显示
- Daisy Seed CPU 足够单 App 内 FFT + FX，但多 App 并行增加路由复杂度

## 后果

- 不能同时跑 Spectrum + Delay；用户需切换
- AppShell 接口清晰，后续可评估 v2 双 App 若硬件升级
