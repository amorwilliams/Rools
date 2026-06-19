#pragma once

#include <cstdint>

namespace rools {

struct Color565 {
    uint16_t v;

    constexpr explicit Color565(uint16_t val = 0) : v(val) {}
};

namespace color {

constexpr Color565 Black   = Color565(0x0000);
constexpr Color565 White   = Color565(0xFFFF);
constexpr Color565 Blue    = Color565(0x001F);
constexpr Color565 Cyan    = Color565(0x07FF);
constexpr Color565 Green   = Color565(0x07E0);
constexpr Color565 Magenta = Color565(0xF81F);
constexpr Color565 Yellow  = Color565(0xFFE0);
constexpr Color565 Orange  = Color565(0xFD20);
constexpr Color565 Red     = Color565(0xF800);
constexpr Color565 DkGray  = Color565(0x1084);
constexpr Color565 LtGray  = Color565(0xC618);

constexpr Color565 Rgb(uint8_t r, uint8_t g, uint8_t b)
{
    return Color565(static_cast<uint16_t>(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)));
}

} // namespace color

class St7735;

class Gfx {
public:
    enum class LineQuality : uint8_t {
        Fast,
        AntiAliased,
    };

    explicit Gfx(St7735& lcd);

    void Clear(Color565 c);
    void FillRect(int x, int y, int w, int h, Color565 c);
    void DrawRect(int x, int y, int w, int h, Color565 c);
    void DrawHLine(int x, int y, int w, Color565 c);
    void DrawVLine(int x, int y, int h, Color565 c);
    void DrawLine(int x0, int y0, int x1, int y1, Color565 c);
    void DrawString(int x, int y, const char* text, Color565 fg, Color565 bg);
    void SetLineQuality(LineQuality quality) { line_quality_ = quality; }
    LineQuality line_quality() const { return line_quality_; }

    void Flush();
    bool FlushRect(int x, int y, int w, int h);
    bool IsBusy() const;

    static constexpr int kWidth  = 160;
    static constexpr int kHeight = 128;

private:
    void SetPixel(int x, int y, Color565 c);
    void SetPixelBlend(int x, int y, Color565 fg, float alpha);
    void DrawLineFast(int x0, int y0, int x1, int y1, Color565 c);
    void DrawLineAA(int x0, int y0, int x1, int y1, Color565 c);
    void MarkDirtyRect(int x, int y, int w, int h);

    St7735& lcd_;
    bool    dirty_ = false;
    int     dirty_x0_ = 0;
    int     dirty_y0_ = 0;
    int     dirty_x1_ = 0;
    int     dirty_y1_ = 0;
    LineQuality line_quality_ = LineQuality::Fast;
};

} // namespace rools
