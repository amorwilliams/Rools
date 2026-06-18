#pragma once

#include <cstddef>
#include <cstdint>

namespace rools {

enum class GestureType : uint8_t {
    EncTurnA,
    EncTurnB,
    ShiftEncTurnA,
    ShiftEncTurnB,
    EncPressA,
    EncReleaseA,
    EncPressB,
    EncReleaseB,
    CenterTap,
    ShiftEnter,
    ShiftExit,
};

struct GestureEvent {
    GestureType type;
    int32_t     value = 0;
};

struct GestureFrameInput {
    uint32_t now_ms;
    int32_t  enc_a_delta;
    int32_t  enc_b_delta;
    uint8_t  a_events;
    uint8_t  b_events;
    uint8_t  c_events;
    bool     btn_center_pressed;
    bool     enc_a_pressed;
};

struct GestureFrameResult {
    static constexpr size_t kMaxEvents = 16;
    GestureEvent            events[kMaxEvents];
    size_t                  count        = 0;
    bool                    shift_active = false;
    bool                    ui_dirty     = false;

    void Push(GestureType type, int32_t value = 0)
    {
        if(count >= kMaxEvents)
            return;
        events[count++] = GestureEvent{type, value};
    }
};

class InputGestureController {
public:
    static constexpr uint32_t kShiftHoldMs = 220;
    static constexpr uint32_t kMenuHoldMs  = 500;
    static constexpr uint32_t kRepeatMs    = 80;

    GestureFrameResult Update(const GestureFrameInput& in);

private:
    bool     shift_pressed_  = false;
    uint32_t btn_down_ms_    = 0;
    bool     btn_long_fired_ = false;
};

} // namespace rools
