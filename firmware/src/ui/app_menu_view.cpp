#include "ui/app_menu_view.h"

#include "display/gfx.h"
#include "display/theme.h"

namespace rools {

void DrawAppMenuView(Gfx& g, size_t selected, size_t count, const char* (*name_fn)(size_t))
{
    const auto& t = theme::kDefault;
    const int   w = Gfx::kWidth;
    const int   h = Gfx::kHeight;
    const int   row_h = 14;
    const int   list_y = 20;

    g.Clear(t.bg);
    g.DrawString(2, 2, "Apps", t.accent, t.bg);

    for(size_t i = 0; i < count; ++i)
    {
        const int  y   = list_y + static_cast<int>(i) * row_h;
        const bool sel = (i == selected);
        if(sel)
            g.FillRect(0, y - 1, w, row_h, t.border);

        const Color565 fg = sel ? t.fg : t.muted;
        const Color565 bg = sel ? t.border : t.bg;
        g.DrawString(6, y, name_fn(i), fg, bg);
    }

    g.DrawHLine(0, h - 12, w, t.border);
    g.DrawString(2, h - 10, "A:OK B:Back", t.muted, t.bg);
    g.Flush();
}

} // namespace rools
