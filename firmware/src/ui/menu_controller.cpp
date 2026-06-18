#include "ui/menu_controller.h"

namespace rools {

MenuResult MenuController::Update(const MenuUpdateInput& in)
{
    MenuResult out;

    for(size_t i = 0; i < in.event_count; ++i)
    {
        const GestureEvent& e = in.events[i];
        if(e.type == GestureType::EncPressA)
        {
            enc_a_down_ms_    = in.now_ms;
            enc_a_long_fired_ = false;
        }
    }

    if(!open_ && in.enc_a_pressed)
    {
        if(in.now_ms - enc_a_down_ms_ >= kLongPressMs && !enc_a_long_fired_)
        {
            open_             = true;
            selected_         = in.current_app_index;
            enc_a_long_fired_ = true;
            out.ui_dirty      = true;
        }
    }

    if(open_)
    {
        out.consumed = true;

        for(size_t i = 0; i < in.event_count; ++i)
        {
            const GestureEvent& e = in.events[i];
            if(e.type == GestureType::EncTurnA && in.app_count > 0)
            {
                int sel = static_cast<int>(selected_) + static_cast<int>(e.value);
                while(sel < 0)
                    sel += static_cast<int>(in.app_count);
                selected_   = static_cast<size_t>(sel % static_cast<int>(in.app_count));
                out.ui_dirty = true;
            }
            else if(e.type == GestureType::EncReleaseA)
            {
                if(enc_a_long_fired_)
                {
                    enc_a_long_fired_ = false;
                }
                else
                {
                    out.request_switch = true;
                    out.switch_index   = selected_;
                    open_              = false;
                    out.ui_dirty       = true;
                }
            }
            else if(e.type == GestureType::EncReleaseB)
            {
                open_        = false;
                out.ui_dirty = true;
            }
        }
    }

    out.menu_open = open_;
    out.selected  = selected_;
    return out;
}

} // namespace rools
