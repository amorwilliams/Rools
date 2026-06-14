# 08 — App 丝滑切换（MFX 式）

> 参考：ALM MFX 多效果器切换无卡顿、无 Pop。Rools 多 App 切换目标对齐此体验。

## 现状（M1）

| 能力 | 状态 |
|------|------|
| 音频 ISR / UI 主循环分离 | ✅ 已实现 |
| `App` 基类 + `AppRegistry` | ✅ 已实现 |
| `load_app()` 立即切换 | ✅ 仅 `init()` 启动用 |
| UI 递 flag、音频侧切换 | 🔲 接口 `request_app_switch()` |
| Crossfade 防 Pop | 🔲 接口 `AppSwitchPhase` + `fade_gain_` |
| 共享 DSP 静态内存池 | 🔲 接口 `DspMemoryPool` |
| 参数保存 / 恢复 | 🔲 各 App static 实例暂保留；正式 `on_suspend` 后续 |

详见 `firmware/src/app_shell.h`、`firmware/src/dsp/dsp_memory_pool.h`。

## 目标架构

```
┌─────────────────┐         pending_index_          ┌──────────────────┐
│  UI Main Loop   │ ─── request_app_switch() ───► │  Audio ISR       │
│  Enc / 菜单     │         (只写 flag)            │  TickSwitch…()   │
│  ui_draw 30fps  │                               │  fade → Commit   │
└─────────────────┘                               └──────────────────┘
```

### 1. UI 与音频彻底分离

- **UI 线程**：读 Encoder、画菜单、调用 `request_app_switch(index)`，**不**直接改 `current_`。
- **音频 ISR**：每 buffer 开头 `TickSwitchStateMachine()`，见 flag 后自行完成切换。

### 2. Crossfade 消除 Pop

切换状态机：

```
Idle → FadingOut (~10 ms) → Switching → FadingIn (~10 ms) → Idle
```

- **FadingOut**：对输出乘 `fade_gain_` 1→0。
- **Switching**（静音点）：`on_exit()` → `DspMemoryPool::Reset()` → 换 App → `on_enter()`。
- **FadingIn**：`fade_gain_` 0→1。

默认 ~10 ms（48 kHz 下约 480 sample），可配置 `kSwitchFadeMs`。

### 3. 静态 DSP 内存池

Delay / Reverb 等 FX 需要大块 RAM，禁止 ISR 内 `malloc`。

- 开机分配 **一块最大 delay line**（`DspMemoryPool`，SDRAM）。
- 各 FX App 从 pool **bump 分配**，不 `free`。
- 切换 App 时 **`memset` 整池清零**，避免上一 App 尾音泄漏。
- 使用 pool 的 App 覆写 `uses_shared_dsp_memory()` / `on_release_shared_memory()`。

### 4. 状态保存与恢复

- 分析类 App（Spectrum）：参数存在 static 实例成员，切走再切回自然保留。
- FX App：必要时在 `on_exit()` 写 Flash preset 或 shell 侧 slot 表（M3+）。
- 菜单 UI：`AppRegistry::Name(i)` 列表 + Enc 选择 → `request_app_switch(i)`。

## 代码接口（已预留）

```cpp
// UI 线程
shell.request_app_switch(index);

// App 可选钩子
virtual bool uses_shared_dsp_memory() const;
virtual void on_release_shared_memory();

// 共享 RAM
DspMemoryPool pool;  // AppShell 成员，FX App 向 shell 申请
```

## 实现里程碑

| 阶段 | 任务 |
|------|------|
| **M2** | App 菜单 UI + `request_app_switch` 接线 |
| **M3** | Fade 状态机 + `CommitAppSwitch` 在 ISR 完成 |
| **M3** | `DspMemoryPool` 实现 + Delay/Reverb 接入 |
| **M4** | Preset JSON 与切换联动 |

## 与 ADR-001 关系

[ADR-001](decisions/ADR-001-single-app-mode.md) 单 App **运行**模式不变：同一时刻仍只有一个 App 占音频链；切换流程变丝滑，不是多 App 并行混音。
