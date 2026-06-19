#include "ui/layout_view.h"

#include <cstdio>
#include <cstring>

#include "display/theme.h"
#include "ui/bottom_bar_view.h"

namespace rools {

LayoutView::LayoutView(Gfx& gfx)
: gfx_(gfx)
{
}

void LayoutView::ResetCache()
{
    last_top_app_      = nullptr;
    last_bottom_app_   = nullptr;
    last_shift_active_ = false;
    last_top_[0]       = '\0';
    last_left_[0]      = '\0';
    last_mid_[0]       = '\0';
    last_right_[0]     = '\0';
}

const char* LayoutView::NormalizeHint(const char* in, char* out, size_t out_size) const
{
    if(!out || out_size == 0)
        return "";
    out[0] = '\0';
    if(!in || in[0] == '\0')
        return out;

    const char* p = in;
    if((p[0] == 'A' || p[0] == 'B') && p[1] == ':')
        p += 2;
    else if((p[0] == 'B' || p[0] == 'b') && (p[1] == 't' || p[1] == 'T') && (p[2] == 'n' || p[2] == 'N')
            && p[3] == ':')
        p += 4;
    else if((p[0] == 'C' || p[0] == 'c') && std::strncmp(p, "Center:", 7) == 0)
        p += 7;
    while(*p == ' ')
        ++p;

    std::snprintf(out, out_size, "%s", p);
    return out;
}

bool LayoutView::DrawTopIfDirty(const App* app)
{
    if(!app)
        return false;

    const auto& t = theme::kDefault;
    const char* top_src = nullptr;
    const bool  use_text = app->current_top_hint(gfx_, top_src);
    if(!use_text)
    {
        // App 已通过 current_top_hint(gfx, ...) 自绘顶栏。
        last_top_[0]  = '\0';
        last_top_app_ = app;
        return true;
    }

    if(!top_src || top_src[0] == '\0')
        top_src = app->name();

    const bool dirty = (last_top_app_ != app) || std::strncmp(last_top_, top_src, sizeof(last_top_)) != 0;
    if(!dirty)
        return false;

    gfx_.FillRect(0, 0, Gfx::kWidth, kTopBarHeight, t.bg);
    gfx_.DrawString(2, 2, top_src, t.accent, t.bg);
    std::snprintf(last_top_, sizeof(last_top_), "%.31s", top_src);
    last_top_app_ = app;
    return true;
}

bool LayoutView::DrawBottomIfDirty(const App* app, bool shift_active)
{
    if(!app)
        return false;

    char left[24];
    char mid[24];
    char right[24];

    NormalizeHint(app->current_a_hint(), left, sizeof(left));
    const char* button = shift_active ? app->current_button_shift_hint() : app->current_button_hint();
    if(shift_active && (!button || button[0] == '\0'))
        button = app->current_shift_hint();
    NormalizeHint(button, mid, sizeof(mid));
    NormalizeHint(app->current_b_hint(), right, sizeof(right));

    DrawBottomBarView(gfx_,
                      (left[0] != '\0') ? left : "-",
                      (mid[0] != '\0') ? mid : "-",
                      (right[0] != '\0') ? right : "-");
    last_bottom_app_   = app;
    last_shift_active_ = shift_active;
    std::snprintf(last_left_, sizeof(last_left_), "%.23s", left);
    std::snprintf(last_mid_, sizeof(last_mid_), "%.23s", mid);
    std::snprintf(last_right_, sizeof(last_right_), "%.23s", right);
    return true;
}

bool LayoutView::RenderAppFrame(App* app, bool shift_active)
{
    if(!app)
        return false;

    const bool top_dirty    = DrawTopIfDirty(app);
    app->ui_draw(metrics_);
    const bool bottom_dirty = DrawBottomIfDirty(app, shift_active);

    int flush_y0 = metrics_.main_top;
    int flush_y1 = metrics_.main_bottom;
    if(top_dirty)
        flush_y0 = 0;
    if(bottom_dirty)
        flush_y1 = metrics_.main_bottom + metrics_.bottom_height;
    gfx_.FlushRect(0, flush_y0, Gfx::kWidth, flush_y1 - flush_y0);
    return true;
}

} // namespace rools
