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

    void ui_draw() override;
    void on_enc(Enc enc, int delta) override;
    void on_btn(Btn btn, bool pressed) override;

    const ParamMap* param_map() const override;

private:
    void DrawPattern(int id);
    void DrawInputDebug();

    Gfx* gfx_ = nullptr;
    int  pattern_ = 0;
    bool debug_on_ = true;
};

} // namespace rools
