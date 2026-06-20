#pragma once

#include "daisy_seed.h"

// Rools Core Seed map — grouped for PCB routing (see seed.kicad_sch).
// Dn == GPIOn on symbol; physical pin number != D (audio/power pads in between).

namespace rools {
namespace pins {

using Pin = daisy::Pin;

// Enc A — D0–D2, Seed pins 1–3
constexpr Pin kEncA_A  = daisy::seed::D0;
constexpr Pin kEncA_B  = daisy::seed::D1;
constexpr Pin kEncA_Sw = daisy::seed::D2;

// Btn + ST7735 SPI — D4–D10, Seed pins 5–11
constexpr Pin kBtnCenter = daisy::seed::D4;
constexpr Pin kLcdBlk    = daisy::seed::D5; // SD_CMD
constexpr Pin kLcdRst    = daisy::seed::D6; // SD_CK
constexpr Pin kLcdCs     = daisy::seed::D7; // SPI_CS
constexpr Pin kLcdSck    = daisy::seed::D8; // SPI_SCK
constexpr Pin kLcdDc     = daisy::seed::D9; // SPI_MISO
constexpr Pin kLcdMosi   = daisy::seed::D10; // SPI_MOSI

// I2C (codec + MCP4728) — D11–D12, Seed pins 12–13 — wired on Audio sheet
constexpr Pin kI2cScl = daisy::seed::D11;
constexpr Pin kI2cSda = daisy::seed::D12;

// CV in (K+jack sum) — D15–D18, Seed pins 22–25
constexpr Pin kCv1Adc = daisy::seed::D15;
constexpr Pin kCv2Adc = daisy::seed::D16;
constexpr Pin kCv3Adc = daisy::seed::D17;
constexpr Pin kCv4Adc = daisy::seed::D18;

// Enc B — D21–D23, Seed pins 28–30
constexpr Pin kEncB_A  = daisy::seed::D21;
constexpr Pin kEncB_B  = daisy::seed::D22;
constexpr Pin kEncB_Sw = daisy::seed::D23;

// SAI2 (PCM3060) — D24–D28, Seed pins 31–35 — wired on Audio sheet

// TODO(M2 PCB): kPwrFail = daisy::seed::D13; // EXTI, comparator on +5V hold-up

} // namespace pins
} // namespace rools
