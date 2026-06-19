#pragma once

#include <cstddef>
#include <cstdint>

#include "input/input_gesture_controller.h"

namespace rools {

class Gfx;

struct MenuUpdateInput {
    uint32_t now_ms;
    bool     enc_a_pressed;
    size_t   app_count;
    size_t   current_app_index;
    const GestureEvent* events;
    size_t              event_count;
};

struct MenuResult {
    bool   consumed       = false;
    bool   request_switch = false;
    size_t switch_index   = 0;
    bool   menu_open      = false;
    size_t selected       = 0;
    bool   ui_dirty       = false;
};

class AppMenuView {
public:
    static constexpr uint32_t kLongPressMs = InputGestureController::kMenuHoldMs;

    MenuResult Update(const MenuUpdateInput& in);
    void Draw(Gfx& g, size_t count, const char* (*name_fn)(size_t)) const;

private:
    bool     open_             = false;
    size_t   selected_         = 0;
    uint32_t enc_a_down_ms_    = 0;
    bool     enc_a_long_fired_ = false;
};

} // namespace rools
