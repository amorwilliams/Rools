#pragma once

#include <cstdint>

#include "app_shell.h"
#include "display/gfx.h"

namespace rools {

/** CV 两点(0V/+5V)逐通道校准 App,结果写入统一设置层。 */
class CalibrationApp : public App {
public:
    void Bind(Gfx* gfx);

    const char* name() const override { return "Calib"; }

    void on_enter() override;
    void on_exit() override;

    void audio_callback(const float* inL,
                        const float* inR,
                        float*       outL,
                        float*       outR,
                        size_t       n) override;

    void ui_draw(const LayoutMetrics& layout) override;
    void on_enc(Enc enc, int delta) override;
    void on_btn(Btn btn, bool pressed) override;

    const ParamMap* param_map() const override;
    const char*     current_a_hint() const override { return "-"; }
    const char*     current_b_hint() const override { return "Channel"; }
    const char*     current_button_hint() const override;

private:
    enum class Stage : uint8_t { SelectCh, Apply0V, Apply5V, Done };

    void  RunSample(int point);          // 阻塞:Sampling -> 采样 -> Done! -> 下一态
    float SampleLong(size_t ch);         // ~5s 时间分散平均,带进度
    void  ShowMsg(const char* msg, Color565 color);
    void  ComputeAndSave();

    Gfx*     gfx_           = nullptr;
    Stage    stage_         = Stage::SelectCh;
    int      sel_ch_        = 0;
    float    uni0_          = 0.f;
    float    uni5_          = 0.f;
    float    center_        = 0.f;
    float    atten_         = 0.f;
    uint32_t last_action_ms_ = 0;        // 按键锁定,防采样后误触
};

} // namespace rools
