#pragma once

#include "app_shell.h"
#include "display/gfx.h"

namespace rools {

struct LayoutMetrics {
    int top_height;
    int bottom_height;
    int main_top;
    int main_bottom;
    int main_height;
};

class LayoutView {
public:
    static constexpr int kTopBarHeight    = 12;
    static constexpr int kBottomBarHeight = 10;
    static constexpr int kMainTop         = kTopBarHeight;
    static constexpr int kMainBottom      = Gfx::kHeight - kBottomBarHeight;

    explicit LayoutView(Gfx& gfx);

    void ResetCache();
    bool DrawTopIfDirty(const App* app);
    bool DrawBottomIfDirty(const App* app, bool shift_active);
    bool RenderAppFrame(App* app, bool shift_active);
    const LayoutMetrics& metrics() const { return metrics_; }

private:
    const char* NormalizeHint(const char* in, char* out, size_t out_size) const;

    Gfx&      gfx_;
    const App* last_top_app_    = nullptr;
    const App* last_bottom_app_ = nullptr;
    bool      last_shift_active_ = false;
    char      last_top_[32]    = {0};
    char      last_left_[24]   = {0};
    char      last_mid_[24]    = {0};
    char      last_right_[24]  = {0};
    LayoutMetrics metrics_{kTopBarHeight, kBottomBarHeight, kMainTop, kMainBottom, kMainBottom - kMainTop};
};

} // namespace rools
