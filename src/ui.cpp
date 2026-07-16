#include "ui.h"
#include "config.h"
#include "font_data.h"

#include <vector>

namespace ui {
namespace {
Font font{};
bool ready = false;

// Press Start 2P is an 8px-grid font; drawing at multiples keeps it crisp.
float Spacing(int size) { return (float)size / 8.0f; }
} // namespace

void Init() {
    std::vector<int> cps;
    for (int c = 32; c <= 126; c++) cps.push_back(c);          // ASCII
    cps.push_back(0x401);                                       // Ё
    for (int c = 0x410; c <= 0x44F; c++) cps.push_back(c);      // А..я
    cps.push_back(0x451);                                       // ё
    cps.push_back(0x2014);                                      // —

    font = LoadFontFromMemory(".ttf", FONT_PS2P, (int)FONT_PS2P_LEN,
                              32, cps.data(), (int)cps.size());
    if (font.texture.id != 0) {
        SetTextureFilter(font.texture, TEXTURE_FILTER_POINT);
        ready = true;
    }
}

void Shutdown() {
    if (ready) UnloadFont(font);
    ready = false;
}

void Text(const char* text, int x, int y, int size, Color color) {
    if (!ready) { DrawText(text, x, y, size, color); return; }
    DrawTextEx(font, text, { (float)x, (float)y }, (float)size, Spacing(size), color);
}

int Measure(const char* text, int size) {
    if (!ready) return MeasureText(text, size);
    return (int)MeasureTextEx(font, text, (float)size, Spacing(size)).x;
}

void TextCentered(const char* text, int y, int size, Color color) {
    Text(text, cfg::RENDER_W / 2 - Measure(text, size) / 2, y, size, color);
}

} // namespace ui
