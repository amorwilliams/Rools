#pragma once

#include "app_shell.h"
#include "display/gfx.h"

namespace rools {

class DisplayTestApp : public App {
public:
    void Bind(Gfx* gfx);

    const char* name() const override { return "DisplayTest"; }

    void on_enter() override;
    void on_exit() override;

    void audio_callback(const float* inL,
                        const float* inR,
                        float*       outL,
                        float*       outR,
                        size_t       n) override;

    void ui_draw(const LayoutMetrics& layout) override;
    void on_enc(Enc enc, int delta) override;
    void on_enc_shift(Enc enc, int delta) override;
    void on_enc_press_shift(Enc enc, bool pressed) override;
    void on_btn(Btn btn, bool pressed) override;

    const ParamMap* param_map() const override;
    const char*     current_a_hint() const override { return "None"; }
    const char*     current_b_hint() const override { return "Pattern"; }
    const char*     current_button_shift_hint() const override { return "Debug"; }
    const char*     current_shift_hint() const override { return "Debug"; }

private:
    void DrawPattern(int id, const LayoutMetrics& layout);
    void DrawInputDebug(const LayoutMetrics& layout);

    Gfx* gfx_ = nullptr;
    int  pattern_ = 0;
    bool debug_on_ = true;
};

} // namespace rools
