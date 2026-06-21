#include "board/pwr_fail.h"

#include "board/pins.h"
#include "display/st7735.h"
#include "settings/settings_store.h"

#include "per/gpio.h"
#include "per/tim.h"
#include "sys/system.h"

namespace rools {

namespace {

using daisy::GPIO;
using daisy::System;
using daisy::TimerHandle;

GPIO        g_pwr_fail;
TimerHandle g_timer;
uint8_t     g_low_streak  = 0;
bool        g_triggered   = false;

constexpr uint8_t kLowConfirmMs = 5;

void OnTimer(void*)
{
    if(g_triggered)
        return;

    if(!g_pwr_fail.Read())
    {
        if(g_low_streak < kLowConfirmMs)
            ++g_low_streak;
        if(g_low_streak >= kLowConfirmMs)
        {
            g_triggered = true;
            EmergencyBacklightOff();
            SettingsStore::OnPowerFail();
        }
    }
    else
    {
        g_low_streak = 0;
    }
}

} // namespace

void PwrFail::Init()
{
    GPIO::Config gcfg;
    gcfg.pin  = pins::kPwrFail;
    gcfg.mode = GPIO::Mode::INPUT;
    gcfg.pull = GPIO::Pull::NOPULL;
    g_pwr_fail.Init(gcfg);

    TimerHandle::Config tcfg;
    tcfg.periph      = TimerHandle::Config::Peripheral::TIM_3;
    tcfg.period      = 999;
    tcfg.enable_irq  = true;
    g_timer.Init(tcfg);

    const uint32_t tim_hz = System::GetPClk1Freq() * 2;
    g_timer.SetPrescaler(tim_hz / 1000000 - 1);
    g_timer.SetCallback(OnTimer, nullptr);
    g_timer.Start();
}

} // namespace rools
