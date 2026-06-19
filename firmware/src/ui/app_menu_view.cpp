#include "ui/app_menu_view.h"

#include "display/gfx.h"
#include "display/theme.h"
#include "ui/bottom_bar_view.h"

namespace rools {

MenuResult AppMenuView::Update(const MenuUpdateInput& in)
{
    MenuResult out;

    for(size_t i = 0; i < in.event_count; ++i)
    {
        const GestureEvent& e = in.events[i];
        if(e.type == GestureType::EncPressA)
        {
            enc_a_down_ms_    = in.now_ms;
            enc_a_long_fired_ = false;
        }
    }

    if(!open_ && in.enc_a_pressed)
    {
        if(in.now_ms - enc_a_down_ms_ >= kLongPressMs && !enc_a_long_fired_)
        {
            open_             = true;
            selected_         = in.current_app_index;
            enc_a_long_fired_ = true;
            out.ui_dirty      = true;
        }
    }

    if(open_)
    {
        out.consumed = true;
        for(size_t i = 0; i < in.event_count; ++i)
        {
            const GestureEvent& e = in.events[i];
            if(e.type == GestureType::EncTurnA && in.app_count > 0)
            {
                int sel = static_cast<int>(selected_) + static_cast<int>(e.value);
                while(sel < 0)
                    sel += static_cast<int>(in.app_count);
                selected_    = static_cast<size_t>(sel % static_cast<int>(in.app_count));
                out.ui_dirty = true;
            }
            else if(e.type == GestureType::EncReleaseA)
            {
                if(enc_a_long_fired_)
                {
                    enc_a_long_fired_ = false;
                }
                else
                {
                    out.request_switch = true;
                    out.switch_index   = selected_;
                    open_              = false;
                    out.ui_dirty       = true;
                }
            }
            else if(e.type == GestureType::EncReleaseB)
            {
                open_        = false;
                out.ui_dirty = true;
            }
        }
    }

    out.menu_open = open_;
    out.selected  = selected_;
    return out;
}

void AppMenuView::Draw(Gfx& g, size_t count, const char* (*name_fn)(size_t)) const
{
    const auto& t = theme::kDefault;
    const int   w = Gfx::kWidth;
    const int   row_h = 14;
    const int   list_y = 20;

    g.Clear(t.bg);
    g.DrawString(2, 2, "Apps", t.accent, t.bg);

    for(size_t i = 0; i < count; ++i)
    {
        const int  y   = list_y + static_cast<int>(i) * row_h;
        const bool sel = (i == selected_);
        if(sel)
            g.FillRect(0, y - 1, w, row_h, t.border);

        const Color565 fg = sel ? t.fg : t.muted;
        const Color565 bg = sel ? t.border : t.bg;
        g.DrawString(6, y, name_fn(i), fg, bg);
    }

    DrawBottomBarView(g, "OK", "", "Back");
    g.Flush();
}

} // namespace rools
