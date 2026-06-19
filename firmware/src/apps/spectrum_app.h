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

    void ui_draw(const LayoutMetrics& layout) override;

    void on_enc(Enc enc, int delta) override;
    void on_enc_press(Enc enc, bool pressed) override;
    void on_enc_shift(Enc enc, int delta) override;
    void on_btn(Btn btn, bool pressed) override;

    const ParamMap* param_map() const override;
    const char*     current_a_hint() const override;
    const char*     current_b_hint() const override;
    const char*     current_button_hint() const override { return freeze_latched_ ? "Unfreeze" : "Freeze"; }
    const char*     current_button_shift_hint() const override { return "Full"; }
    const char*     current_shift_hint() const override { return "Full"; }

    FftAnalyzer& analyzer() { return analyzer_; }

private:
    enum class FocusParam : uint8_t {
        PeakTrack,
        Cursor,
        Freeze,
        Gain,
        Decay,
        FftSize,
        PeakHold,
        Count
    };

    static constexpr size_t kRingSize = 2048;

    Gfx*                  gfx_ = nullptr;
    FftAnalyzer           analyzer_;
    SampleRing<kRingSize> ring_;
    SpectrumView          view_;

    float gain_db_          = 0.f;
    float peak_hold_decay_  = 0.65f;
    bool  peak_hold_        = false;
    bool  fullscreen_       = false;
    bool  fine_mode_        = false;
    bool  peak_track_enabled_ = true;
    bool  cursor_enabled_   = true;
    bool  freeze_enabled_   = false;
    bool  freeze_latched_   = false;
    size_t cursor_bin_      = 0;
    FocusParam focus_param_ = FocusParam::PeakTrack;
    float in_peak_          = 0.f;
    float in_rms_           = 0.f;
    mutable char b_hint_[32]{};
};

} // namespace rools
