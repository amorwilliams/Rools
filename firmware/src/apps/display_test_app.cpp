#include "apps/display_test_app.h"

#include "board/enc_debug.h"
#include "display/theme.h"
#include "ui/layout_view.h"

#include <cstdio>

namespace rools {

namespace {

void DrawDebugBar(Gfx* gfx, int y, int bottom, const InputDebugSnapshot& d, int pattern)
{
    const auto& t = theme::kDefault;
    char        buf[40];

    gfx->FillRect(0, y, Gfx::kWidth, bottom - y, t.bg);
    gfx->DrawHLine(0, y, Gfx::kWidth, t.border);

    snprintf(buf,
             sizeof(buf),
             "A d:%+d sw:%d ev:%u",
             d.enc_a_delta,
             d.enc_a_sw ? 1 : 0,
             static_cast<unsigned>(d.enc_a_evt));
    gfx->DrawString(2, y + 2, buf, t.fg, t.bg);

    snprintf(buf,
             sizeof(buf),
             "B d:%+d sw:%d P:%d",
             d.enc_b_delta,
             d.enc_b_sw ? 1 : 0,
             pattern);
    gfx->DrawString(2, y + 11, buf, t.muted, t.bg);

    snprintf(buf,
             sizeof(buf),
             "Btn:%d ev:%u",
             d.btn_center ? 1 : 0,
             static_cast<unsigned>(d.btn_evt));
    gfx->DrawString(2, y + 20, buf, t.accent, t.bg);
}

} // namespace

void DisplayTestApp::Bind(Gfx* gfx)
{
    gfx_ = gfx;
}

void DisplayTestApp::on_enter()
{
    pattern_  = 0;
    debug_on_ = false;
}

void DisplayTestApp::on_exit() {}

void DisplayTestApp::audio_callback(const float* inL,
                                    const float* inR,
                                    float*       outL,
                                    float*       outR,
                                    size_t       n)
{
    for(size_t i = 0; i < n; ++i)
    {
        outL[i] = inL[i];
        outR[i] = inR[i];
    }
}

void DisplayTestApp::DrawPattern(int id, const LayoutMetrics& layout)
{
    const int w = Gfx::kWidth;
    const int top = layout.main_top;
    const int h = layout.main_bottom - layout.main_top;

    const auto& t = theme::kDefault;
    gfx_->FillRect(0, top, w, h, t.bg);

    switch(id % 3)
    {
    case 0:
    {
        gfx_->DrawString(2, top + 2, "Midnight", t.accent, t.bg);
        gfx_->DrawString(2, top + 12, "theme", t.fg, t.bg);
        gfx_->FillRect(4, top + 28, w - 8, 4, t.accent);
        gfx_->FillRect(4, top + 36, w - 8, 4, t.border);
        gfx_->FillRect(4, top + 44, w - 8, 4, t.muted);
        gfx_->FillRect(4, top + 52, w - 8, 4, t.fg);
        const int bar_w = (w - 8) / 3;
        gfx_->FillRect(4, top + 64, bar_w, 40, t.bar_low);
        gfx_->FillRect(4 + bar_w, top + 64, bar_w, 40, t.bar_mid);
        gfx_->FillRect(4 + bar_w * 2, top + 64, bar_w, 40, t.bar_high);
        break;
    }
    case 1:
    {
        const int sz = 8;
        for(int y = top; y < top + h; y += sz)
        {
            for(int x = 0; x < w; x += sz)
            {
                const bool on = ((x / sz) + ((y - top) / sz)) & 1;
                gfx_->FillRect(x, y, sz, sz, on ? t.border : t.bg);
            }
        }
        gfx_->DrawString(2, top + 2, "Grid 1", t.accent, t.bg);
        break;
    }
    default:
        gfx_->DrawRect(0, top, w, h, t.border);
        gfx_->DrawHLine(0, top + h / 2, w, t.accent);
        gfx_->DrawVLine(w / 2, top, h, t.accent);
        gfx_->DrawString(2, top + 2, "Cross 2", t.fg, t.bg);
        break;
    }

}

void DisplayTestApp::DrawInputDebug(const LayoutMetrics& layout)
{
    if(!gfx_)
        return;

    DrawDebugBar(gfx_, layout.main_bottom - 32, layout.main_bottom, InputDebug(), pattern_);
}

void DisplayTestApp::ui_draw(const LayoutMetrics& layout)
{
    if(!gfx_)
        return;

    DrawPattern(pattern_, layout);
    if(debug_on_)
        DrawInputDebug(layout);
}

void DisplayTestApp::on_btn(Btn btn, bool pressed)
{
    (void)btn;
    (void)pressed;
}

void DisplayTestApp::on_enc(Enc enc, int delta)
{
    if(enc != Enc::B || delta == 0)
        return;

    pattern_ += delta;
    if(pattern_ < 0)
        pattern_ = 2;
    if(pattern_ > 2)
        pattern_ = 0;
}

void DisplayTestApp::on_enc_shift(Enc enc, int delta)
{
    (void)enc;
    (void)delta;
}

void DisplayTestApp::on_enc_press_shift(Enc enc, bool pressed)
{
    if(enc != Enc::A || !pressed)
        return;
    debug_on_ = !debug_on_;
}

const ParamMap* DisplayTestApp::param_map() const
{
    static ParamMap map{"-", "-", "-", "Pattern"};
    return &map;
}

} // namespace rools
