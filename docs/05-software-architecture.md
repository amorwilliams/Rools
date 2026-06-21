# 05 — 软件架构

## 技术栈

- [libDaisy](https://daisy.audio/software/) + DaisySP
- C++17
- 48 kHz stereo audio callback（float，Seed 板载 24-bit）

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
| UIRenderer | ST7735 菜单/波形/频谱；**SPI DMA + 局部刷新**（[ADR-012](decisions/ADR-012-display-spi-dma.md)） |
| IOMapper | 8 路 ADC（4 CV + 4 Knob）→ 4 列 `ControlColumn`（`knob`/`cv`/`sum` 软件合成） |
| CVRouter | CV A–D 输出分配 → `CvOutDriver`（DAC8565 SPI1 + LDAC） |
| ClockSync | CV3/CV4 边沿检测（App 可选） |
| UsbStorage | FAT32、样本/预设/OTA（HAS_EXP） |
| SettingsStore | `GlobalSettings` QSPI 环缓持久化；见 [ADR-015](decisions/ADR-015-settings-persistence.md) |

## 设置持久化

- `SettingsStore`：`Init` 读环缓 / 迁移旧格式 → RAM；`MarkDirty` + `Tick` 节流落盘；`Flush` 强制写
- CV 校准、切 App 等关键路径显式 `Flush`
- 掉电保存：D28 1 kHz debounce → 关背光 → `OnPowerFail()` / `Flush()`（[ADR-015](decisions/ADR-015-settings-persistence.md)）

## 输入采集约束（必须遵守）

- Encoder / Switch 的 `Debounce()` 必须在固定频率中断里执行（当前用 `AudioCallback`）。
- 中断里只做采集与累计：读取 `Increment()` 并累加到共享计数器，记录按下/抬起事件位。
- 主循环只做消费：原子读取并清零累计值，再分发到 `on_enc / on_btn`。
- 禁止在 UI 主循环里直接 `Debounce()`；SPI 刷屏会阻塞主循环，导致漏采样和手感变差。

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
Seed onboard In (L,R) → [耦合/Mono] → App::audio_callback → [耦合/Mono out] → Seed onboard Out
                                      ↑
                              columns[0..3] knob/cv/sum（控制率，AudioCallback 更新）
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
