#include "apps/calibration_app.h"

#include "board/cv_calibration.h"
#include "display/theme.h"
#include "settings/settings_store.h"
#include "sys/system.h"
#include "ui/layout_view.h"

#include <cstdio>

namespace rools {

namespace {

constexpr int      kSampleCount   = 5000; // ~5s (面包板噪声大,加长平均)
constexpr int      kProgressStep  = 500;  // 每 0.5s 刷新进度
constexpr uint32_t kDoneHoldMs   = 500;
constexpr uint32_t kBtnLockoutMs = 400;

void Fmt3(char* dst, size_t n, float v)
{
    bool neg = v < 0.f;
    if(neg)
        v = -v;
    int scaled = static_cast<int>(v * 1000.f + 0.5f);
    snprintf(dst, n, "%s%d.%03d", neg ? "-" : "", scaled / 1000, scaled % 1000);
}

} // namespace

void CalibrationApp::Bind(Gfx* gfx)
{
    gfx_ = gfx;
}

void CalibrationApp::on_enter()
{
    stage_          = Stage::SelectCh;
    uni0_           = 0.f;
    uni5_           = 0.f;
    center_         = 0.f;
    atten_          = 0.f;
    last_action_ms_ = 0;
}

void CalibrationApp::on_exit() {}

void CalibrationApp::audio_callback(const float* inL,
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

float CalibrationApp::SampleLong(size_t ch)
{
    float       acc = 0.f;
    const auto& t   = theme::kDefault;
    const int   top = LayoutView::kMainTop;
    const int   bot = LayoutView::kMainBottom;
    char        buf[24];

    for(int i = 0; i < kSampleCount; ++i)
    {
        acc += GetCvAdcRaw(ch);
        daisy::System::Delay(1);

        if(gfx_ && (i % kProgressStep == 0 || i == kSampleCount - 1))
        {
            const int pct = ((i + 1) * 100 + kSampleCount - 1) / kSampleCount;
            snprintf(buf, sizeof(buf), "Sampling %d%%", pct);
            gfx_->FillRect(0, top, Gfx::kWidth, bot - top, t.bg);
            gfx_->DrawString(36, top + (bot - top) / 2 - 4, buf, t.accent, t.bg);
            gfx_->Flush();
        }
    }
    return acc / static_cast<float>(kSampleCount);
}

void CalibrationApp::ShowMsg(const char* msg, Color565 color)
{
    if(!gfx_)
        return;
    const auto& t   = theme::kDefault;
    const int   top = LayoutView::kMainTop;
    const int   bot = LayoutView::kMainBottom;
    gfx_->FillRect(0, top, Gfx::kWidth, bot - top, t.bg);
    gfx_->DrawString(48, top + (bot - top) / 2 - 4, msg, color, t.bg);
    gfx_->Flush();
}

void CalibrationApp::ComputeAndSave()
{
    center_ = uni0_ * kCvAdcRefVolts;
    float d = (uni0_ - uni5_) * kCvAdcRefVolts / 5.0f;
    atten_  = d < 0.f ? -d : d; // 幅值
    if(atten_ < 0.001f)
        atten_ = kCvCalibrationDefault.attenuation;
    SetCvChannelCal(static_cast<size_t>(sel_ch_), CvCalibration{center_, atten_});
    SettingsStore::Instance().Flush(); // 校准数据立即持久化
}

void CalibrationApp::RunSample(int point)
{
    const auto& t = theme::kDefault;

    ShowMsg("Sampling...", t.accent);
    const float avg = SampleLong(static_cast<size_t>(sel_ch_));

    if(point == 0)
    {
        uni0_  = avg;
        stage_ = Stage::Apply5V;
    }
    else
    {
        uni5_ = avg;
        ComputeAndSave();
        stage_ = Stage::Done;
    }

    ShowMsg("Done!", t.bar_high);
    daisy::System::Delay(kDoneHoldMs);
    last_action_ms_ = daisy::System::GetNow(); // 阻塞结束后重置锁定基准
}

void CalibrationApp::on_enc(Enc enc, int delta)
{
    (void)enc;
    if(stage_ != Stage::SelectCh || delta == 0)
        return;
    sel_ch_ += delta;
    while(sel_ch_ < 0)
        sel_ch_ += 4;
    while(sel_ch_ > 3)
        sel_ch_ -= 4;
}

void CalibrationApp::on_btn(Btn btn, bool pressed)
{
    (void)btn;
    if(!pressed)
        return;

    const uint32_t now = daisy::System::GetNow();
    if(now - last_action_ms_ < kBtnLockoutMs)
        return; // 防误触
    last_action_ms_ = now;

    switch(stage_)
    {
    case Stage::SelectCh: stage_ = Stage::Apply0V; break;
    case Stage::Apply0V: RunSample(0); break;
    case Stage::Apply5V: RunSample(1); break;
    case Stage::Done: stage_ = Stage::SelectCh; break;
    }
}

const char* CalibrationApp::current_button_hint() const
{
    switch(stage_)
    {
    case Stage::SelectCh: return "Start";
    case Stage::Apply0V: return "OK";
    case Stage::Apply5V: return "OK";
    case Stage::Done: return "Again";
    }
    return "OK";
}

void CalibrationApp::ui_draw(const LayoutMetrics& layout)
{
    if(!gfx_)
        return;

    const auto& t   = theme::kDefault;
    const int   x   = 4;
    const int   top = layout.main_top;
    int         y   = top + 2;
    char        buf[40];

    gfx_->FillRect(0, top, Gfx::kWidth, layout.main_bottom - top, t.bg);

    gfx_->DrawString(x, y, "CV CALIBRATION", t.accent, t.bg);
    y += 12;

    snprintf(buf, sizeof(buf), "Channel: CV%d", sel_ch_ + 1);
    gfx_->DrawString(x, y, buf, t.fg, t.bg);
    y += 12;

    char v[16];
    Fmt3(v, sizeof(v), GetCvAdcRaw(static_cast<size_t>(sel_ch_)));
    snprintf(buf, sizeof(buf), "raw: %s", v);
    gfx_->DrawString(x, y, buf, t.muted, t.bg);
    y += 14;

    switch(stage_)
    {
    case Stage::SelectCh:
        gfx_->DrawString(x, y, "Turn: pick channel", t.fg, t.bg);
        gfx_->DrawString(x, y + 10, "Press OK: start", t.muted, t.bg);
        break;
    case Stage::Apply0V:
        gfx_->DrawString(x, y, "Patch 0V", t.fg, t.bg);
        gfx_->DrawString(x, y + 10, "Press OK", t.muted, t.bg);
        break;
    case Stage::Apply5V:
        gfx_->DrawString(x, y, "Patch +5V", t.fg, t.bg);
        gfx_->DrawString(x, y + 10, "Press OK", t.muted, t.bg);
        break;
    case Stage::Done:
    {
        char c[16];
        char a[16];
        Fmt3(c, sizeof(c), center_);
        Fmt3(a, sizeof(a), atten_);
        snprintf(buf, sizeof(buf), "Saved CV%d", sel_ch_ + 1);
        gfx_->DrawString(x, y, buf, t.accent, t.bg);
        snprintf(buf, sizeof(buf), "C=%s A=%s", c, a);
        gfx_->DrawString(x, y + 10, buf, t.fg, t.bg);
        break;
    }
    }
}

const ParamMap* CalibrationApp::param_map() const
{
    static ParamMap map{"CH", "-", "-", "-"};
    return &map;
}

} // namespace rools
