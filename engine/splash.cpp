#include "splash.h"
#include "bloodengine.h"
#include "engine_config.h"
#include "ui.h"
#include "raylib.h"

#include <cmath>
#include <cstdio>

namespace {
constexpr Color BLOOD  = { 230, 30, 40, 255 };
constexpr Color BLOOD_D = { 140, 16, 24, 255 };
constexpr Color ACCENT = { 255, 230, 0, 255 };
constexpr Color FAINT  = { 120, 60, 64, 255 };

// smooth 0..1 ramp between a and b
float Smooth(float t, float a, float b) {
    float x = (t - a) / (b - a);
    x = x < 0 ? 0 : (x > 1 ? 1 : x);
    return x * x * (3 - 2 * x);
}
} // namespace

void Splash::Start() { t_ = 0; }

void Splash::Update(float dt) {
    if (!Finished()) t_ += dt;
}

void Splash::Draw() const {
    const int W = cfg::RENDER_W, H = cfg::RENDER_H;
    const Vector2 c = { W / 2.0f, H / 2.0f - 60 };

    float fadeIn = Smooth(t_, 0.15f, 0.8f);
    float fadeOut = 1.0f - Smooth(t_, DURATION - 0.5f, DURATION - 0.1f);
    float a = fadeIn * fadeOut;

    // rotating gear ring with teeth
    float spin = t_ * 24.0f;
    DrawRing(c, 92, 108, 0, 360, 48, Fade(BLOOD_D, a));
    for (int i = 0; i < 10; i++) {
        float ang = spin + i * 36.0f;
        Rectangle tooth = { c.x, c.y, 26, 26 };
        Vector2 origin = { 13, 13 + 116 };
        DrawRectanglePro(tooth, origin, ang, Fade(BLOOD_D, a));
    }
    DrawRing(c, 84, 88, 0, 360, 48, Fade(BLOOD, a * 0.8f));

    // blood drop: triangle tip + circle body + highlight
    float pulse = 1.0f + 0.04f * sinf(t_ * 5.0f);
    float r = 40 * pulse;
    DrawCircleV({ c.x, c.y + 14 }, r, Fade(BLOOD, a));
    DrawTriangle({ c.x, c.y - 64 * pulse },
                 { c.x - r * 0.82f, c.y - 4 },
                 { c.x + r * 0.82f, c.y - 4 }, Fade(BLOOD, a));
    DrawCircleV({ c.x - 14, c.y + 2 }, 9, Fade(WHITE, a * 0.55f));

    // engine name + version
    Color nameCol = Fade(BLOOD, a);
    ui::TextCentered(be::NAME, (int)(c.y + 130), 52, nameCol);
    char ver[48];
    snprintf(ver, sizeof(ver), "engine v%s", be::VERSION);
    ui::TextCentered(ver, (int)(c.y + 192), 16, Fade(FAINT, a));
    ui::TextCentered("built on raylib", H - 36, 12, Fade(FAINT, a * 0.8f));

    // loading bar
    float load = Smooth(t_, 0.3f, DURATION - 0.6f);
    int bw = 340, bx = W / 2 - bw / 2, by = (int)(c.y + 240);
    DrawRectangle(bx, by, bw, 8, Fade(BLOOD_D, a * 0.6f));
    DrawRectangle(bx, by, (int)(bw * load), 8, Fade(ACCENT, a));
    if (load < 1.0f)
        ui::TextCentered("LOADING", by + 22, 12, Fade(FAINT, a));
}
