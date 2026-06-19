#include "display/gfx.h"

#include "display/st7735.h"

#include <cmath>

namespace rools {

namespace {

// 5x7 font, ASCII 32..127
static const uint8_t kFont5x7[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x5F, 0x00, 0x00, 0x00, 0x07, 0x00, 0x07, 0x00,
    0x14, 0x7F, 0x14, 0x7F, 0x14, 0x24, 0x2A, 0x7F, 0x2A, 0x12, 0x23, 0x13, 0x08, 0x64, 0x62,
    0x36, 0x49, 0x56, 0x20, 0x50, 0x00, 0x08, 0x07, 0x03, 0x00, 0x00, 0x1C, 0x22, 0x41, 0x00,
    0x00, 0x41, 0x22, 0x1C, 0x00, 0x2A, 0x1C, 0x7F, 0x1C, 0x2A, 0x08, 0x08, 0x3E, 0x08, 0x08,
    0x00, 0x80, 0x70, 0x30, 0x00, 0x08, 0x08, 0x08, 0x08, 0x08, 0x00, 0x00, 0x60, 0x60, 0x00,
    0x20, 0x10, 0x08, 0x04, 0x02, 0x3E, 0x51, 0x49, 0x45, 0x3E, 0x00, 0x42, 0x7F, 0x40, 0x00,
    0x72, 0x49, 0x49, 0x49, 0x46, 0x21, 0x41, 0x49, 0x4D, 0x33, 0x18, 0x14, 0x12, 0x7F, 0x10,
    0x27, 0x45, 0x45, 0x45, 0x39, 0x3C, 0x4A, 0x49, 0x49, 0x31, 0x41, 0x21, 0x11, 0x09, 0x07,
    0x36, 0x49, 0x49, 0x49, 0x36, 0x46, 0x49, 0x49, 0x29, 0x1E, 0x00, 0x00, 0x14, 0x00, 0x00,
    0x00, 0x40, 0x34, 0x00, 0x00, 0x00, 0x08, 0x14, 0x22, 0x41, 0x14, 0x14, 0x14, 0x14, 0x14,
    0x00, 0x41, 0x22, 0x14, 0x08, 0x02, 0x01, 0x59, 0x09, 0x06, 0x3E, 0x41, 0x5D, 0x59, 0x4E,
    0x7C, 0x12, 0x11, 0x12, 0x7C, 0x7F, 0x49, 0x49, 0x49, 0x36, 0x3E, 0x41, 0x41, 0x41, 0x22,
    0x7F, 0x41, 0x41, 0x41, 0x3E, 0x7F, 0x49, 0x49, 0x49, 0x41, 0x7F, 0x09, 0x09, 0x09, 0x01,
    0x3E, 0x41, 0x41, 0x51, 0x73, 0x7F, 0x08, 0x08, 0x08, 0x7F, 0x00, 0x41, 0x7F, 0x41, 0x00,
    0x20, 0x40, 0x41, 0x3F, 0x01, 0x7F, 0x08, 0x14, 0x22, 0x41, 0x7F, 0x40, 0x40, 0x40, 0x40,
    0x7F, 0x02, 0x1C, 0x02, 0x7F, 0x7F, 0x04, 0x08, 0x10, 0x7F, 0x3E, 0x41, 0x41, 0x41, 0x3E,
    0x7F, 0x09, 0x09, 0x09, 0x06, 0x3E, 0x41, 0x51, 0x21, 0x5E, 0x7F, 0x09, 0x19, 0x29, 0x46,
    0x26, 0x49, 0x49, 0x49, 0x32, 0x03, 0x01, 0x7F, 0x01, 0x03, 0x3F, 0x40, 0x40, 0x40, 0x3F,
    0x1F, 0x20, 0x40, 0x20, 0x1F, 0x3F, 0x60, 0x18, 0x60, 0x3F, 0x63, 0x14, 0x08, 0x14, 0x63,
    0x03, 0x04, 0x78, 0x04, 0x03, 0x61, 0x59, 0x49, 0x4D, 0x43, 0x00, 0x7F, 0x41, 0x41, 0x41,
    0x02, 0x04, 0x08, 0x10, 0x20, 0x00, 0x41, 0x41, 0x41, 0x7F, 0x04, 0x02, 0x01, 0x02, 0x04,
    0x40, 0x40, 0x40, 0x40, 0x40, 0x00, 0x03, 0x07, 0x08, 0x00, 0x20, 0x54, 0x54, 0x78, 0x40,
    0x7F, 0x28, 0x44, 0x44, 0x38, 0x38, 0x44, 0x44, 0x44, 0x28, 0x38, 0x44, 0x44, 0x28, 0x7F,
    0x38, 0x54, 0x54, 0x54, 0x18, 0x00, 0x08, 0x7E, 0x09, 0x02, 0x18, 0xA4, 0xA4, 0x9C, 0x78,
    0x7F, 0x08, 0x04, 0x04, 0x78, 0x00, 0x44, 0x7D, 0x40, 0x00, 0x20, 0x40, 0x40, 0x3D, 0x00,
    0x7F, 0x10, 0x28, 0x44, 0x00, 0x00, 0x41, 0x7F, 0x40, 0x00, 0x7C, 0x04, 0x78, 0x04, 0x78,
    0x7C, 0x08, 0x04, 0x04, 0x78, 0x38, 0x44, 0x44, 0x44, 0x38, 0xFC, 0x18, 0x24, 0x24, 0x18,
    0x18, 0x24, 0x24, 0x18, 0xFC, 0x7C, 0x08, 0x04, 0x04, 0x08, 0x48, 0x54, 0x54, 0x54, 0x24,
    0x04, 0x04, 0x3F, 0x44, 0x24, 0x3C, 0x40, 0x40, 0x20, 0x7C, 0x1C, 0x20, 0x40, 0x20, 0x1C,
    0x3C, 0x40, 0x30, 0x40, 0x3C, 0x44, 0x28, 0x10, 0x28, 0x44, 0x4C, 0x90, 0x90, 0x90, 0x7C,
    0x44, 0x64, 0x54, 0x4C, 0x44, 0x00, 0x08, 0x36, 0x41, 0x00, 0x00, 0x00, 0x77, 0x00, 0x00,
    0x00, 0x41, 0x36, 0x08, 0x00, 0x02, 0x01, 0x02, 0x04, 0x02,
};

inline uint8_t Blend8(uint8_t bg, uint8_t fg, uint8_t alpha)
{
    const int out = (static_cast<int>(bg) * (255 - alpha) + static_cast<int>(fg) * alpha + 127) / 255;
    return static_cast<uint8_t>(out);
}

inline float Frac(float v)
{
    return v - std::floor(v);
}

inline float RFrac(float v)
{
    return 1.0f - Frac(v);
}

} // namespace

Gfx::Gfx(St7735& lcd)
: lcd_(lcd)
{
}

void Gfx::MarkDirtyRect(int x, int y, int w, int h)
{
    if(w <= 0 || h <= 0)
        return;

    int x0 = x;
    int y0 = y;
    int x1 = x + w;
    int y1 = y + h;
    if(x0 < 0)
        x0 = 0;
    if(y0 < 0)
        y0 = 0;
    if(x1 > kWidth)
        x1 = kWidth;
    if(y1 > kHeight)
        y1 = kHeight;
    if(x1 <= x0 || y1 <= y0)
        return;

    if(!dirty_)
    {
        dirty_    = true;
        dirty_x0_ = x0;
        dirty_y0_ = y0;
        dirty_x1_ = x1;
        dirty_y1_ = y1;
        return;
    }

    if(x0 < dirty_x0_)
        dirty_x0_ = x0;
    if(y0 < dirty_y0_)
        dirty_y0_ = y0;
    if(x1 > dirty_x1_)
        dirty_x1_ = x1;
    if(y1 > dirty_y1_)
        dirty_y1_ = y1;
}

void Gfx::SetPixel(int x, int y, Color565 c)
{
    if(x < 0 || y < 0 || x >= kWidth || y >= kHeight)
        return;
    lcd_.framebuffer()[y * kWidth + x] = c.v;
    MarkDirtyRect(x, y, 1, 1);
}

void Gfx::SetPixelBlend(int x, int y, Color565 fg, float alpha)
{
    if(x < 0 || y < 0 || x >= kWidth || y >= kHeight)
        return;
    if(alpha <= 0.0f)
        return;
    if(alpha >= 1.0f)
    {
        SetPixel(x, y, fg);
        return;
    }

    uint16_t* fb = lcd_.framebuffer();
    uint16_t  bg = fb[y * kWidth + x];

    const uint8_t br = static_cast<uint8_t>((bg >> 11) & 0x1F);
    const uint8_t bgc = static_cast<uint8_t>((bg >> 5) & 0x3F);
    const uint8_t bb = static_cast<uint8_t>(bg & 0x1F);
    const uint8_t fr = static_cast<uint8_t>((fg.v >> 11) & 0x1F);
    const uint8_t fgc = static_cast<uint8_t>((fg.v >> 5) & 0x3F);
    const uint8_t fbv = static_cast<uint8_t>(fg.v & 0x1F);

    const uint8_t a8 = static_cast<uint8_t>(alpha * 255.0f);
    const uint8_t or5 = Blend8(br, fr, a8);
    const uint8_t og6 = Blend8(bgc, fgc, a8);
    const uint8_t ob5 = Blend8(bb, fbv, a8);

    fb[y * kWidth + x] = static_cast<uint16_t>((or5 << 11) | (og6 << 5) | ob5);
    MarkDirtyRect(x, y, 1, 1);
}

void Gfx::Clear(Color565 c)
{
    uint16_t* fb = lcd_.framebuffer();
    for(int i = 0; i < kWidth * kHeight; ++i)
        fb[i] = c.v;
    MarkDirtyRect(0, 0, kWidth, kHeight);
}

void Gfx::FillRect(int x, int y, int w, int h, Color565 c)
{
    if(w <= 0 || h <= 0)
        return;

    int x0 = x;
    int y0 = y;
    int x1 = x + w;
    int y1 = y + h;
    if(x0 < 0)
        x0 = 0;
    if(y0 < 0)
        y0 = 0;
    if(x1 > kWidth)
        x1 = kWidth;
    if(y1 > kHeight)
        y1 = kHeight;
    if(x1 <= x0 || y1 <= y0)
        return;

    uint16_t* fb     = lcd_.framebuffer();
    const int width  = x1 - x0;
    const int height = y1 - y0;
    for(int row = 0; row < height; ++row)
    {
        uint16_t* dst = &fb[(y0 + row) * kWidth + x0];
        for(int col = 0; col < width; ++col)
            dst[col] = c.v;
    }
    MarkDirtyRect(x0, y0, width, height);
}

void Gfx::DrawRect(int x, int y, int w, int h, Color565 c)
{
    DrawHLine(x, y, w, c);
    DrawHLine(x, y + h - 1, w, c);
    DrawVLine(x, y, h, c);
    DrawVLine(x + w - 1, y, h, c);
}

void Gfx::DrawHLine(int x, int y, int w, Color565 c)
{
    if(w <= 0 || y < 0 || y >= kHeight)
        return;
    int x0 = x;
    int x1 = x + w;
    if(x0 < 0)
        x0 = 0;
    if(x1 > kWidth)
        x1 = kWidth;
    if(x1 <= x0)
        return;
    uint16_t* dst = &lcd_.framebuffer()[y * kWidth + x0];
    for(int i = 0; i < x1 - x0; ++i)
        dst[i] = c.v;
    MarkDirtyRect(x0, y, x1 - x0, 1);
}

void Gfx::DrawVLine(int x, int y, int h, Color565 c)
{
    if(h <= 0 || x < 0 || x >= kWidth)
        return;
    int y0 = y;
    int y1 = y + h;
    if(y0 < 0)
        y0 = 0;
    if(y1 > kHeight)
        y1 = kHeight;
    if(y1 <= y0)
        return;
    uint16_t* fb = lcd_.framebuffer();
    for(int i = y0; i < y1; ++i)
        fb[i * kWidth + x] = c.v;
    MarkDirtyRect(x, y0, 1, y1 - y0);
}

void Gfx::DrawLine(int x0, int y0, int x1, int y1, Color565 c)
{
    if(line_quality_ == LineQuality::AntiAliased)
    {
        DrawLineAA(x0, y0, x1, y1, c);
        return;
    }
    DrawLineFast(x0, y0, x1, y1, c);
}

void Gfx::DrawLineFast(int x0, int y0, int x1, int y1, Color565 c)
{
    if(y0 == y1)
    {
        const int w = (x0 <= x1) ? (x1 - x0 + 1) : (x0 - x1 + 1);
        DrawHLine((x0 <= x1) ? x0 : x1, y0, w, c);
        return;
    }
    if(x0 == x1)
    {
        const int h = (y0 <= y1) ? (y1 - y0 + 1) : (y0 - y1 + 1);
        DrawVLine(x0, (y0 <= y1) ? y0 : y1, h, c);
        return;
    }

    int dx = x1 - x0;
    if(dx < 0)
        dx = -dx;
    int sx = (x0 < x1) ? 1 : -1;

    int dy = y1 - y0;
    if(dy < 0)
        dy = -dy;
    dy = -dy;
    int sy = (y0 < y1) ? 1 : -1;

    int err = dx + dy;
    while(true)
    {
        SetPixel(x0, y0, c);
        if(x0 == x1 && y0 == y1)
            break;
        const int e2 = err * 2;
        if(e2 >= dy)
        {
            err += dy;
            x0 += sx;
        }
        if(e2 <= dx)
        {
            err += dx;
            y0 += sy;
        }
    }
}

void Gfx::DrawLineAA(int x0, int y0, int x1, int y1, Color565 c)
{
    if(y0 == y1 || x0 == x1)
    {
        DrawLineFast(x0, y0, x1, y1, c);
        return;
    }

    bool steep = std::abs(y1 - y0) > std::abs(x1 - x0);
    if(steep)
    {
        const int tx0 = x0;
        x0            = y0;
        y0            = tx0;
        const int tx1 = x1;
        x1            = y1;
        y1            = tx1;
    }
    if(x0 > x1)
    {
        const int tx = x0;
        x0           = x1;
        x1           = tx;
        const int ty = y0;
        y0           = y1;
        y1           = ty;
    }

    const float dx       = static_cast<float>(x1 - x0);
    const float dy       = static_cast<float>(y1 - y0);
    const float gradient = (dx == 0.0f) ? 1.0f : (dy / dx);

    float xend = std::round(static_cast<float>(x0));
    float yend = static_cast<float>(y0) + gradient * (xend - static_cast<float>(x0));
    float xgap = RFrac(static_cast<float>(x0) + 0.5f);
    int   xpxl1 = static_cast<int>(xend);
    int   ypxl1 = static_cast<int>(std::floor(yend));
    if(steep)
    {
        SetPixelBlend(ypxl1, xpxl1, c, RFrac(yend) * xgap);
        SetPixelBlend(ypxl1 + 1, xpxl1, c, Frac(yend) * xgap);
    }
    else
    {
        SetPixelBlend(xpxl1, ypxl1, c, RFrac(yend) * xgap);
        SetPixelBlend(xpxl1, ypxl1 + 1, c, Frac(yend) * xgap);
    }
    float intery = yend + gradient;

    xend = std::round(static_cast<float>(x1));
    yend = static_cast<float>(y1) + gradient * (xend - static_cast<float>(x1));
    xgap = Frac(static_cast<float>(x1) + 0.5f);
    int xpxl2 = static_cast<int>(xend);
    int ypxl2 = static_cast<int>(std::floor(yend));
    if(steep)
    {
        SetPixelBlend(ypxl2, xpxl2, c, RFrac(yend) * xgap);
        SetPixelBlend(ypxl2 + 1, xpxl2, c, Frac(yend) * xgap);
    }
    else
    {
        SetPixelBlend(xpxl2, ypxl2, c, RFrac(yend) * xgap);
        SetPixelBlend(xpxl2, ypxl2 + 1, c, Frac(yend) * xgap);
    }

    for(int x = xpxl1 + 1; x < xpxl2; ++x)
    {
        const int yi = static_cast<int>(std::floor(intery));
        if(steep)
        {
            SetPixelBlend(yi, x, c, RFrac(intery));
            SetPixelBlend(yi + 1, x, c, Frac(intery));
        }
        else
        {
            SetPixelBlend(x, yi, c, RFrac(intery));
            SetPixelBlend(x, yi + 1, c, Frac(intery));
        }
        intery += gradient;
    }
}

void Gfx::DrawString(int x, int y, const char* text, Color565 fg, Color565 bg)
{
    int cx = x;
    while(text && *text)
    {
        const char ch = *text++;
        if(ch < 32 || ch > 127)
            continue;

        const uint8_t* glyph = &kFont5x7[(ch - 32) * 5];
        for(int col = 0; col < 5; ++col)
        {
            const uint8_t line = glyph[col];
            for(int row = 0; row < 7; ++row)
            {
                const bool on = (line >> row) & 0x01;
                SetPixel(cx + col, y + row, on ? fg : bg);
            }
        }
        cx += 6;
    }
}

void Gfx::Flush()
{
    if(!dirty_ || IsBusy())
        return;

    if(FlushRect(dirty_x0_, dirty_y0_, dirty_x1_ - dirty_x0_, dirty_y1_ - dirty_y0_))
        dirty_ = false;
}

bool Gfx::FlushRect(int x, int y, int w, int h)
{
    if(IsBusy())
        return false;
    return lcd_.StartWriteFramebufferRect(x, y, w, h);
}

bool Gfx::IsBusy() const
{
    return lcd_.IsBusy();
}

} // namespace rools
