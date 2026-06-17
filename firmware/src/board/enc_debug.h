#pragma once

#include <cstdint>

namespace rools {

struct InputDebugSnapshot {
    int     enc_a_delta = 0;
    int     enc_b_delta = 0;
    uint8_t enc_a_evt   = 0;
    uint8_t enc_b_evt   = 0;
    uint8_t btn_evt     = 0;
    bool    enc_a_sw    = false;
    bool    enc_b_sw    = false;
    bool    btn_center  = false;
};

const InputDebugSnapshot& InputDebug();

void InputDebugUpdate(int     enc_a_delta,
                      int     enc_b_delta,
                      uint8_t enc_a_evt,
                      uint8_t enc_b_evt,
                      uint8_t btn_evt,
                      bool    enc_a_sw,
                      bool    enc_b_sw,
                      bool    btn_center);

} // namespace rools
