# 05 — 软件架构

## 技术栈

- [libDaisy](https://daisy.audio/software/) + DaisySP
- C++17
- 48 kHz stereo audio callback（float，Codec 24-bit）

## 单 App 模式

```
Boot → AppShell → AppMenu → [当前 App]
                              ↓
                    on_enter / process / on_exit
```

切换：Enc A 长按 → 选 App → 确认 → **~10 ms crossfade** → 新 App `on_enter()`。

丝滑切换设计详见 **[08-app-switching.md](08-app-switching.md)**（MFX 式：UI 递 flag、ISR fade、静态 DSP 池）。

## AppShell 职责

| 模块 | 职责 |
|------|------|
| AudioEngine | I/O、Mono 复制、AC/DC 高通、callback 分发 |
| UIRenderer | ST7735 菜单/波形/频谱 |
| IOMapper | K1–K4 + CV1–CV4 列映射 |
| CVRouter | CV A–D 输出分配 |
| ClockSync | CV3/CV4 边沿检测（App 可选） |
| UsbStorage | FAT32、样本/预设/OTA（HAS_EXP） |

## App 接口（见 firmware/src/app_shell.h）

```cpp
class App {
  virtual void on_enter() = 0;
  virtual void on_exit() = 0;
  virtual void audio_callback(float* inL, float* inR,
                              float* outL, float* outR, size_t n) = 0;
  virtual void ui_draw() = 0;
  virtual void on_encoder(Enc enc, int delta) {}
  virtual void on_button(Button btn) {}
  virtual const ParamMap* params() const = 0;
};
```

## 音频流

```
Codec In (L,R) → [耦合/Mono] → App::audio_callback → [耦合/Mono out] → Codec Out
                                      ↑
                              K1–K4, CV1–CV4 (控制率)
```

## USB 目录（Exp）

```
/rools/
├── samples/
├── presets/
├── scenes/
├── _updater/     # rools_firmware.bin, update.log
└── README.html   # v2
```

## 条件编译

| 宏 | 含义 |
|----|------|
| `HAS_EXP` | 装 Exp：MIDI + USB Host |
| `HAS_USB` | 同 HAS_EXP |

## App 路线图

| 优先级 | App | 类型 |
|--------|-----|------|
| P0 | SpectrumAnalyzer | 可视化 |
| P0 | Oscilloscope | 可视化 |
| P1 | Lissajous | 可视化 |
| P1 | Delay, Reverb | FX |
| P2 | Granular, EnvelopeFollower | FX / 工具 |
