#pragma once

#include <cstddef>
#include <cstdint>

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
 * 所有 App 的基类。AppShell 在 audio callback 中调用 audio_callback。
 */
class App {
public:
    virtual ~App() = default;

    virtual const char* name() const = 0;

    virtual void on_enter() = 0;
    virtual void on_exit() = 0;

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
 * 全局壳：音频 I/O、UI、App 切换、CV 路由。
 * M1 实现于 app_shell.cpp
 */
class AppShell {
public:
    void init();
    void run_forever(); // blocks; calls Daisy inner loop

    bool load_app(size_t index);
    size_t app_count() const;

    /** 四列输入（K + CV 已合并为 sum） */
    ControlColumn columns[4];

    CvOutputs cv_out;

    CouplingMode in_coupling  = CouplingMode::DC;
    CouplingMode out_coupling = CouplingMode::AC_10Hz;
    MonoMode     mono_mode    = MonoMode::Auto;

private:
    App* current_ = nullptr;
    void audio_cb_internal(const float* inL, const float* inR, float* outL, float* outR, size_t n);
    void apply_mono_out(float* outL, float* outR, size_t n);
};

} // namespace rools
