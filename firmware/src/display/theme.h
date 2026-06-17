#pragma once

#include "display/gfx.h"

namespace rools {
namespace theme {

struct Palette {
    Color565 bg;
    Color565 fg;
    Color565 muted;
    Color565 accent;
    Color565 border;
    Color565 bar_low;
    Color565 bar_mid;
    Color565 bar_high;
    Color565 peak;

    constexpr Palette(Color565 bg_,
                      Color565 fg_,
                      Color565 muted_,
                      Color565 accent_,
                      Color565 border_,
                      Color565 bar_low_,
                      Color565 bar_mid_,
                      Color565 bar_high_,
                      Color565 peak_)
    : bg(bg_)
    , fg(fg_)
    , muted(muted_)
    , accent(accent_)
    , border(border_)
    , bar_low(bar_low_)
    , bar_mid(bar_mid_)
    , bar_high(bar_high_)
    , peak(peak_)
    {}
};

// Cursor Dark Midnight 风格
inline constexpr Palette kDefault(
    Color565(0x18E4), // bg  #1a1d23
    Color565(0xDE9D), // fg  #d8dee9
    Color565(0x6B90), // muted
    Color565(0x855F), // accent
    Color565(0x2988), // border
    Color565(0x57EA), // bar_low
    Color565(0xE6C5), // bar_mid
    Color565(0xF2A6), // bar_high
    Color565(0xDE9D)  // peak
);

} // namespace theme
} // namespace rools
