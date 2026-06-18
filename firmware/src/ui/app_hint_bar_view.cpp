#include "ui/app_hint_bar_view.h"

#include "display/gfx.h"
#include "display/theme.h"

namespace rools {

void DrawAppHintBarView(Gfx&       g,
                        const char* a_hint,
                        const char* b_hint,
                        bool        shift_active,
                        const char* shift_hint)
{
    const auto& t = theme::kDefault;
    const int   y = Gfx::kHeight - 10;

    const char* a = (a_hint && a_hint[0] != '\0') ? a_hint : "A: -";
    const char* b = (b_hint && b_hint[0] != '\0') ? b_hint : "B: Param";

    g.FillRect(0, y, Gfx::kWidth, 10, t.bg);
    g.DrawHLine(0, y, Gfx::kWidth, t.border);
    if(shift_active)
    {
        const char* s = (shift_hint && shift_hint[0] != '\0') ? shift_hint : "Shift";
        g.DrawString(2, y + 1, s, t.muted, t.bg);
    }
    else
    {
        g.DrawString(2, y + 1, a, t.muted, t.bg);
        g.DrawString(92, y + 1, b, t.muted, t.bg);
    }
}

} // namespace rools
