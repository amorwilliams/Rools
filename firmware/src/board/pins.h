#pragma once

#include "daisy_seed.h"

// M1 breadboard pin map — change here only before PCB.

namespace rools {
namespace pins {

using Pin = daisy::Pin;

// ST7735 SPI (landscape 160x128)
constexpr Pin kLcdSck  = daisy::seed::D8;
constexpr Pin kLcdMosi = daisy::seed::D10;
constexpr Pin kLcdCs   = daisy::seed::D7;
constexpr Pin kLcdDc   = daisy::seed::D9;
constexpr Pin kLcdRst  = daisy::seed::D6;
constexpr Pin kLcdBlk  = daisy::seed::D5;

// Enc A (libDaisy Encoder example pins)
constexpr Pin kEncA_A    = daisy::seed::D20;
constexpr Pin kEncA_B    = daisy::seed::D16;
constexpr Pin kEncA_Sw   = daisy::seed::D19;

// Enc B
constexpr Pin kEncB_A    = daisy::seed::D22;
constexpr Pin kEncB_B    = daisy::seed::D23;
constexpr Pin kEncB_Sw   = daisy::seed::D21;

// Center button (O_C style, between encoders)
constexpr Pin kBtnCenter = daisy::seed::D4;

} // namespace pins
} // namespace rools
