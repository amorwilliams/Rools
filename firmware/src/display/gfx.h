#pragma once

#include <cstdint>

namespace rools {

struct Color565 {
    uint16_t v;
};

namespace color {

constexpr Color565 Black   = {0x0000};
constexpr Color565 White   = {0xFFFF};
constexpr Color565 Blue    = {0x001F};
constexpr Color565 Cyan    = {0x07FF};
constexpr Color565 Green   = {0x07E0};
constexpr Color565 Magenta = {0xF81F};
constexpr Color565 Yellow  = {0xFFE0};
constexpr Color565 Orange  = {0xFD20};
constexpr Color565 Red     = {0xF800};
constexpr Color565 DkGray  = {0x1084};
constexpr Color565 LtGray  = {0xC618};

inline Color565 Rgb(uint8_t r, uint8_t g, uint8_t b)
{
    return {static_cast<uint16_t>(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3))};
}

} // namespace color

class St7735;

class Gfx {
public:
    explicit Gfx(St7735& lcd);

    void Clear(Color565 c);
    void FillRect(int x, int y, int w, int h, Color565 c);
    void DrawRect(int x, int y, int w, int h, Color565 c);
    void DrawHLine(int x, int y, int w, Color565 c);
    void DrawVLine(int x, int y, int h, Color565 c);
    void DrawString(int x, int y, const char* text, Color565 fg, Color565 bg);

    void Flush();

    static constexpr int kWidth  = 160;
    static constexpr int kHeight = 128;

private:
    void SetPixel(int x, int y, Color565 c);

    St7735& lcd_;
};

} // namespace rools
