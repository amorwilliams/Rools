#include "ui/bottom_bar_view.h"

#include <cstdio>

#include "display/gfx.h"
#include "display/theme.h"
#include "ui/layout_view.h"

namespace rools {

void DrawBottomBarView(Gfx& g, const char* left, const char* middle, const char* right)
{
    const auto& t        = theme::kDefault;
    const int   y        = Gfx::kHeight - LayoutView::kBottomBarHeight;
    const int   box_h    = LayoutView::kBottomBarHeight;
    const int   box_w    = Gfx::kWidth;
    const int   seg_w    = box_w / 3;
    const int   split_x1 = seg_w;
    const int   split_x2 = seg_w * 2;
    char        lbuf[16];
    char        mbuf[16];
    char        rbuf[16];
    std::snprintf(lbuf, sizeof(lbuf), "%.8s", (left && left[0] != '\0') ? left : "");
    std::snprintf(mbuf, sizeof(mbuf), "%.8s", (middle && middle[0] != '\0') ? middle : "");
    std::snprintf(rbuf, sizeof(rbuf), "%.8s", (right && right[0] != '\0') ? right : "");

    g.FillRect(0, y, box_w, box_h, t.bg);
    g.DrawRect(0, y, box_w, box_h, t.border);
    g.DrawVLine(split_x1, y, box_h, t.border);
    g.DrawVLine(split_x2, y, box_h, t.border);

    const int text_y = y + 1;
    g.DrawString(2 + (seg_w - 48) / 2, text_y, lbuf, t.muted, t.bg);
    g.DrawString(split_x1 + (seg_w - 48) / 2, text_y, mbuf, t.muted, t.bg);
    g.DrawString(split_x2 + (seg_w - 48) / 2, text_y, rbuf, t.muted, t.bg);
}

} // namespace rools
