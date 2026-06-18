#pragma once

namespace rools {

class Gfx;

void DrawAppHintBarView(Gfx&       g,
                        const char* a_hint,
                        const char* b_hint,
                        bool        shift_active,
                        const char* shift_hint);

} // namespace rools
