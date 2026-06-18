#pragma once

#include <cstddef>

namespace rools {

class Gfx;

void DrawAppMenuView(Gfx& g, size_t selected, size_t count, const char* (*name_fn)(size_t));

} // namespace rools
