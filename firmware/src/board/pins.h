#pragma once

#include "daisy_seed.h"

// Rools Core Seed map — 2026-06 定稿（见 docs/04-hardware-architecture.md）

namespace rools {
namespace pins {

using Pin = daisy::Pin;

// D0 / pin 1 — NC（GPIO0 / USB Host ID 预留，Exp）

// Enc A — D1–D3, Seed pins 2–4
constexpr Pin kEncA_A  = daisy::seed::D1;
constexpr Pin kEncA_B  = daisy::seed::D2;
constexpr Pin kEncA_Sw = daisy::seed::D3;

// Btn + ST7735 SPI — D4–D10, Seed pins 5–11
constexpr Pin kBtnCenter = daisy::seed::D4;
constexpr Pin kLcdBlk    = daisy::seed::D5;
constexpr Pin kLcdRst    = daisy::seed::D6;
constexpr Pin kLcdCs     = daisy::seed::D7;
constexpr Pin kLcdSck    = daisy::seed::D8;
constexpr Pin kLcdDc     = daisy::seed::D9;
constexpr Pin kLcdMosi   = daisy::seed::D10;

// I2C (MCP4728 CV Out) — D11–D12, Seed pins 12–13
constexpr Pin kI2cScl = daisy::seed::D11;
constexpr Pin kI2cSda = daisy::seed::D12;

// CV in — D15–D18, Seed pins 22–25, ADC0–3
constexpr Pin kCv1Adc = daisy::seed::D15;
constexpr Pin kCv2Adc = daisy::seed::D16;
constexpr Pin kCv3Adc = daisy::seed::D17;
constexpr Pin kCv4Adc = daisy::seed::D18;

// Knob — D19–D22, Seed pins 26–29, ADC4–7
constexpr Pin kKnob1Adc = daisy::seed::D19;
constexpr Pin kKnob2Adc = daisy::seed::D20;
constexpr Pin kKnob3Adc = daisy::seed::D21;
constexpr Pin kKnob4Adc = daisy::seed::D22;

// Enc B — D23–D25, Seed pins 30–32
constexpr Pin kEncB_A  = daisy::seed::D23;
constexpr Pin kEncB_B  = daisy::seed::D24;
constexpr Pin kEncB_Sw = daisy::seed::D25;

// D28 / pin 35 — net EXTI_PWR（M2 掉电 EXTI；比较器+保持电容页待画）
// TODO(M2 PCB): constexpr Pin kPwrFail = daisy::seed::D28;

} // namespace pins
} // namespace rools
