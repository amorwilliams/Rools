# App 开发规范

## 新增 App 步骤

1. 在 `apps/` 下创建 `my_app.h` / `my_app.cpp`
2. 继承 `rools::App`，实现全部纯虚函数
3. 在 `main.cpp`（或 `app_registry.cpp`）注册到 `AppShell`
4. 定义 `ParamMap` 标签（K1–K4 列语义）

## 示例骨架

```cpp
class SpectrumApp : public rools::App {
public:
    const char* name() const override { return "Spectrum"; }

    void on_enter() override {}
    void on_exit() override {}

    void audio_callback(const float* inL, const float* inR,
                        float* outL, float* outR, size_t n) override {
        // FFT / passthrough
    }

    void ui_draw() override {}

    const ParamMap* param_map() const override {
        static ParamMap m{"Gain", "Decay", "FFT Size", "Peak Hold"};
        return &m;
    }

    CouplingMode in_coupling() const override { return CouplingMode::DC; }
};
```

## 约束

- `audio_callback` 必须实时安全：无 malloc、无 blocking IO
- UI 在 `ui_draw` 或主循环低优先级刷新，目标 30 fps
- 读 `AppShell::columns[i].cv/sum` 获取统一参考后的 CV（-1..1，见 `board/cv_reference.*`）
- 写 `AppShell::cv_out` 设置 CV A–D（由 shell 送 DAC）

## App 切换（MFX 式）

设计文档：[docs/08-app-switching.md](../../docs/08-app-switching.md)

| API | 线程 | 说明 |
|-----|------|------|
| `request_app_switch(i)` | UI | 运行时切换（TODO M3 fade） |
| `load_app(i)` | 任意 | 立即切换，仅 boot / 调试 |
| `uses_shared_dsp_memory()` | — | FX App 使用 `dsp_pool()` 时覆写 |
| `on_release_shared_memory()` | 音频 | pool Reset 前清尾音 |

```cpp
// TODO(M3) DelayApp 示例
bool uses_shared_dsp_memory() const override { return true; }
void on_release_shared_memory() override { /* 丢弃 delay 状态 */ }
```

## 路线图 App

| 文件（计划） | App |
|-------------|-----|
| `spectrum_app.*` | P0 |
| `oscilloscope_app.*` | P0 |
| `lissajous_app.*` | P1 |
| `delay_app.*` | P1 |
| `reverb_app.*` | P1 |
| `granular_app.*` | P2 |

## USB 依赖

Granular 等需 `HAS_EXP` 与 `UsbStorage`；无 U 盘时使用 Flash factory 样本。
