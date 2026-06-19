#pragma once

#include <cstddef>
#include <cstdint>

#include "dsp/dsp_memory_pool.h"
#include "input/input_gesture_controller.h"
#include "ui/app_menu_view.h"

namespace daisy {
class Encoder;
}

namespace rools {

class Gfx;
struct LayoutMetrics;

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

/** App 切换状态机阶段 — 见 docs/08-app-switching.md */
enum class AppSwitchPhase : uint8_t {
    Idle,
    FadingOut,
    Switching,
    FadingIn,
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
 *   - audio_callback：48 kHz ISR，必须实时安全（无 malloc / blocking / SPI）
 *   - ui_draw / on_enc / on_btn：主循环调用，可访问显示与外设
 *
 * 生命周期（立即切换）：load_app() → on_exit → on_enter
 * 生命周期（丝滑切换）：request_app_switch() → fade → on_exit → on_enter → fade
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

    virtual void ui_draw(const LayoutMetrics& layout) = 0;

    virtual void on_enc(Enc enc, int delta) {}
    virtual void on_enc_shift(Enc enc, int delta) {}
    virtual void on_enc_press(Enc enc, bool pressed) {}
    virtual void on_enc_press_shift(Enc enc, bool pressed) {}
    virtual void on_btn(Btn btn, bool pressed) {}
    virtual void on_btn_shift(Btn btn, bool pressed) {}

    virtual const ParamMap* param_map() const = 0;
    virtual const char*     current_a_hint() const { return nullptr; }
    virtual const char*     current_b_hint() const { return nullptr; }
    virtual const char*     current_button_hint() const { return nullptr; }
    virtual const char*     current_button_shift_hint() const { return nullptr; }
    virtual const char*     current_shift_hint() const { return nullptr; }
    virtual const char*     current_top_hint() const { return nullptr; }
    virtual uint32_t        ui_refresh_interval_ms() const { return 33; }

    /** 默认耦合偏好 */
    virtual CouplingMode in_coupling() const { return CouplingMode::DC; }
    virtual CouplingMode out_coupling() const { return CouplingMode::AC_10Hz; }

    /** FX App 使用 AppShell::dsp_pool() 时返回 true */
    virtual bool uses_shared_dsp_memory() const { return false; }

    /** pool Reset 前调用：静音/丢弃尾音（Delay/Reverb） */
    virtual void on_release_shared_memory() {}
};

/**
 * 全局壳：音频 I/O、UI 刷新、输入转发、App 切换。
 *
 * 主循环 ~1 ms tick，UI 目标 30 fps（33 ms）。
 * M1：init() 用 load_app(0)。多 App 菜单应调用 request_app_switch()。
 *
 * 输入采集规则（关键）：
 *   - 中断采集累计：在 AudioCallback 内 Debounce + Increment 累加 + 事件置位
 *   - 主循环消费清零：run_forever() 原子读出累计值并清零后再分发 on_enc/on_btn
 *   - 禁止在主循环直接 Debounce（刷屏会阻塞采样，导致漏步）
 */
class AppShell {
public:
    static constexpr size_t    kInvalidAppIndex = SIZE_MAX;
    static constexpr float     kSwitchFadeMs    = 10.f;

    void init();
    void run_forever(); // 阻塞；不返回

    /** 由 Daisy 音频 ISR 间接调用 */
    void process_audio(const float* inL,
                       const float* inR,
                       float*       outL,
                       float*       outR,
                       size_t       n);

    /**
     * 立即切换（仅 boot / 调试）。
     * 运行时请用 request_app_switch()。
     */
    bool load_app(size_t index);

    /** UI 线程：请求切换，由音频 ISR 执行 fade + CommitAppSwitch */
    void request_app_switch(size_t index);

    size_t app_count() const;
    size_t current_app_index() const { return current_index_; }
    bool   switch_in_progress() const { return switch_phase_ != AppSwitchPhase::Idle; }
    float  output_fade_gain() const { return fade_gain_; }

    DspMemoryPool& dsp_pool() { return dsp_pool_; }

    /** 四列输入（K + CV 已合并为 sum）— M2 硬件到位后由 shell 填充 */
    ControlColumn columns[4];

    CvOutputs cv_out;

    CouplingMode in_coupling  = CouplingMode::DC;
    CouplingMode out_coupling = CouplingMode::AC_10Hz;
    MonoMode     mono_mode    = MonoMode::Auto;

private:
    App*             current_       = nullptr;
    size_t           current_index_ = kInvalidAppIndex;
    size_t           pending_index_ = kInvalidAppIndex;
    AppSwitchPhase   switch_phase_  = AppSwitchPhase::Idle;
    float            fade_gain_     = 1.f;
    DspMemoryPool    dsp_pool_;
    InputGestureController gesture_controller_;
    AppMenuView      app_menu_view_;

    void audio_cb_internal(const float* inL, const float* inR, float* outL, float* outR, size_t n);
    void apply_mono_out(float* outL, float* outR, size_t n);
    void RouteInputToCurrentApp(const GestureFrameResult& gesture_result, bool& ui_dirty);

    /** 音频 ISR：fade 状态机 + 处理 pending 切换 */
    void TickSwitchStateMachine(size_t num_samples);

    /** 音频 ISR：静音点执行 on_exit / pool Reset / 换 App / on_enter */
    void CommitAppSwitch();

    /** 音频 ISR：对当前 buffer 应用 fade_gain_ */
    void ApplyOutputFade(float* outL, float* outR, size_t n);
};

} // namespace rools
