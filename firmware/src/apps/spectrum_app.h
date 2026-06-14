#pragma once

#include "app_shell.h"
#include "display/gfx.h"
#include "dsp/fft_analyzer.h"
#include "dsp/sample_ring.h"
#include "ui/spectrum_view.h"

namespace rools {

class SpectrumApp : public App {
public:
    void Bind(Gfx* gfx);

    const char* name() const override { return "Spectrum"; }

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

    FftAnalyzer& analyzer() { return analyzer_; }

private:
    static constexpr size_t kRingSize = 2048;

    Gfx*                  gfx_ = nullptr;
    FftAnalyzer           analyzer_;
    SampleRing<kRingSize> ring_;
    SpectrumView          view_;

    float gain_db_    = 0.f;
    float decay_      = 0.65f;
    bool  peak_hold_  = false;
    bool  fullscreen_ = false;
    int   param_idx_  = 0;
};

} // namespace rools
