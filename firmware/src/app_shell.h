#pragma once

#include <cstddef>
#include <cstdint>

namespace daisy {
class Encoder;
}

namespace rools {

enum class Enc : uint8_t { A, B };
enum class Btn : uint8_t { Center };

enum class CouplingMode : uint8_t {
    DC,
    AC_1Hz,
    AC_10Hz,
};

enum class MonoMode : uint8_t {
    Auto,       // 信任 IN_R normalled
    MonoIn,
    StereoIn,
    MonoOut,    // copy L -> R
    StereoOut,
};

/** 一列控制：Knob K + CV In */
struct ControlColumn {
    float knob;  // 0..1 normalized
    float cv;    // -1..1 or 0..1 per App
    float sum;   // hardware-summed equivalent
};

/** CV 输出通道 */
struct CvOutputs {
    float a, b, c, d;
};

/** App 声明的参数语义（每 App 覆盖） */
struct ParamMap {
    const char* col1_label;
    const char* col2_label;
    const char* col3_label;
    const char* col4_label;
};

/**
 * 所有 App 的基类。
 *
 * 线程约定：
 *   - audio_callback：96 kHz ISR，必须实时安全（无 malloc / blocking / SPI）
 *   - ui_draw / on_enc / on_btn：主循环调用，可访问显示与外设
 *
 * 生命周期：load_app() 依次调用 on_exit → on_enter。
 */
class App {
public:
    virtual ~App() = default;

    virtual const char* name() const = 0;

    virtual void on_enter() = 0;
    virtual void on_exit()  = 0;

    virtual void
    audio_callback(const float* inL, const float* inR, float* outL, float* outR, size_t n)
        = 0;

    virtual void ui_draw() = 0;

    virtual void on_enc(Enc enc, int delta) {}
    virtual void on_enc_press(Enc enc, bool pressed) {}
    virtual void on_btn(Btn btn, bool pressed) {}

    virtual const ParamMap* param_map() const = 0;

    /** 默认耦合偏好 */
    virtual CouplingMode in_coupling() const { return CouplingMode::DC; }
    virtual CouplingMode out_coupling() const { return CouplingMode::AC_10Hz; }
};

/**
 * 全局壳：音频 I/O、UI 刷新、输入转发、App 切换。
 *
 * 主循环 ~1 ms tick，UI 目标 30 fps（33 ms）。
 * App 切换接口 load_app() 已就绪；M1 仅启动时 load_app(0)，菜单切换后续迭代。
 */
class AppShell {
public:
    void init();
    void run_forever(); // 阻塞；不返回

    /** 由 Daisy 音频 ISR 间接调用 */
    void process_audio(const float* inL,
                       const float* inR,
                       float*       outL,
                       float*       outR,
                       size_t       n);

    /** 切换 App；失败（index 越界）时保持当前 App */
    bool load_app(size_t index);
    size_t app_count() const;

    /** 四列输入（K + CV 已合并为 sum）— M2 硬件到位后由 shell 填充 */
    ControlColumn columns[4];

    CvOutputs cv_out;

    CouplingMode in_coupling  = CouplingMode::DC;
    CouplingMode out_coupling = CouplingMode::AC_10Hz;
    MonoMode     mono_mode    = MonoMode::Auto;

private:
    App* current_ = nullptr;
    void PollEnc(daisy::Encoder& enc, Enc id);
    void audio_cb_internal(const float* inL, const float* inR, float* outL, float* outR, size_t n);
    void apply_mono_out(float* outL, float* outR, size_t n);
};

} // namespace rools
