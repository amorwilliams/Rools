#include "board/enc_debug.h"

namespace rools {

namespace {

InputDebugSnapshot g_snap;

} // namespace

const InputDebugSnapshot& InputDebug()
{
    return g_snap;
}

void InputDebugUpdate(int     enc_a_delta,
                      int     enc_b_delta,
                      uint8_t enc_a_evt,
                      uint8_t enc_b_evt,
                      uint8_t btn_evt,
                      bool    enc_a_sw,
                      bool    enc_b_sw,
                      bool    btn_center)
{
    g_snap.enc_a_delta = enc_a_delta;
    g_snap.enc_b_delta = enc_b_delta;
    g_snap.enc_a_evt   = enc_a_evt;
    g_snap.enc_b_evt   = enc_b_evt;
    g_snap.btn_evt     = btn_evt;
    g_snap.enc_a_sw    = enc_a_sw;
    g_snap.enc_b_sw    = enc_b_sw;
    g_snap.btn_center  = btn_center;
}

} // namespace rools
