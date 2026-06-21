#pragma once

namespace rools {

/** D28 / EXTI_PWR 掉电检测：1 kHz 定时采样 + 连续 LOW debounce。 */
class PwrFail {
public:
    static void Init();
};

} // namespace rools
