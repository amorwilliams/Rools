#include "input/input_gesture_controller.h"

namespace rools {

namespace {

constexpr uint8_t kEventRise = 0x01;
constexpr uint8_t kEventFall = 0x02;

} // namespace

GestureFrameResult InputGestureController::Update(const GestureFrameInput& in)
{
    GestureFrameResult out;

    if(in.c_events & kEventRise)
    {
        btn_down_ms_    = in.now_ms;
        btn_long_fired_ = false;
    }

    if(in.btn_center_pressed && !in.enc_a_pressed && !btn_long_fired_
       && in.now_ms - btn_down_ms_ >= kShiftHoldMs)
    {
        shift_pressed_  = true;
        btn_long_fired_ = true;
        out.Push(GestureType::ShiftEnter);
        out.ui_dirty = true;
    }

    if(in.c_events & kEventFall)
    {
        if(btn_long_fired_)
        {
            shift_pressed_  = false;
            btn_long_fired_ = false;
            out.Push(GestureType::ShiftExit);
            out.ui_dirty = true;
        }
        else
        {
            out.Push(GestureType::CenterTap);
            out.ui_dirty = true;
        }
    }

    if(in.a_events & kEventRise)
    {
        out.Push(GestureType::EncPressA);
        out.ui_dirty = true;
    }
    if(in.a_events & kEventFall)
    {
        out.Push(GestureType::EncReleaseA);
        out.ui_dirty = true;
    }
    if(in.b_events & kEventRise)
    {
        out.Push(GestureType::EncPressB);
        out.ui_dirty = true;
    }
    if(in.b_events & kEventFall)
    {
        out.Push(GestureType::EncReleaseB);
        out.ui_dirty = true;
    }

    if(in.enc_a_delta != 0)
    {
        out.Push(shift_pressed_ ? GestureType::ShiftEncTurnA : GestureType::EncTurnA, in.enc_a_delta);
        out.ui_dirty = true;
    }
    if(in.enc_b_delta != 0)
    {
        out.Push(shift_pressed_ ? GestureType::ShiftEncTurnB : GestureType::EncTurnB, in.enc_b_delta);
        out.ui_dirty = true;
    }

    out.shift_active = shift_pressed_;
    return out;
}

} // namespace rools
