#include "style_meter.h"
#include "config.h"
#include "ui.h"
#include "raylib.h"

#include <algorithm>
#include <cmath>

namespace {
struct RankDef { const char* name; float threshold; Color color; };
const RankDef RANKS[] = {
    { "D", 0,   { 130, 130, 130, 255 } },
    { "C", 100, { 235, 230, 230, 255 } },
    { "B", 220, { 255, 230, 0, 255 } },
    { "A", 360, { 255, 140, 0, 255 } },
    { "S", 520, { 230, 30, 40, 255 } },
    { "ULTRAVIOLENT", 700, { 255, 40, 90, 255 } },
};
constexpr int NUM_RANKS = 6;
constexpr float MAX_POINTS = 850.0f;
constexpr float POPUP_LIFE = 1.3f;
} // namespace

void StyleMeter::Reset() {
    points_ = 0;
    freshness_ = 0;
    lastKillWeapon_ = -1;
    popups_.clear();
    score = 0;
}

int StyleMeter::RankIndex() const {
    int r = 0;
    for (int i = 1; i < NUM_RANKS; i++)
        if (points_ >= RANKS[i].threshold) r = i;
    return r;
}

void StyleMeter::AddEvent(const char* label, float pts) {
    points_ = std::min(points_ + pts, MAX_POINTS);
    freshness_ = 2.0f;
    popups_.insert(popups_.begin(), { label, POPUP_LIFE });
    if (popups_.size() > 6) popups_.pop_back();
}

void StyleMeter::RegisterKillWeapon(int weaponId) {
    if (lastKillWeapon_ >= 0 && lastKillWeapon_ != weaponId)
        AddEvent("VARIETY", 20);
    lastKillWeapon_ = weaponId;
}

void StyleMeter::Update(float dt) {
    freshness_ -= dt;
    if (freshness_ <= 0) {
        // higher ranks bleed out faster — stay aggressive to keep them
        float drain = 8.0f + RankIndex() * 7.0f;
        points_ = std::max(0.0f, points_ - drain * dt);
    }
    for (auto& p : popups_) p.t -= dt;
    while (!popups_.empty() && popups_.back().t <= 0) popups_.pop_back();
}

void StyleMeter::Draw() const {
    const int W = cfg::RENDER_W, H = cfg::RENDER_H;
    int rank = RankIndex();
    Color col = RANKS[rank].color;

    // ULTRAVIOLENT strobes
    if (rank == NUM_RANKS - 1 && fmodf((float)GetTime() * 6.0f, 1.0f) > 0.5f)
        col = { 255, 230, 0, 255 };

    // rank letter / word (right-aligned: the pixel font is wide)
    const char* name = RANKS[rank].name;
    if (rank == NUM_RANKS - 1) {
        ui::Text("ULTRA", W - 36 - ui::Measure("ULTRA", 22), H / 2 - 88, 22, col);
        ui::Text("VIOLENT", W - 36 - ui::Measure("VIOLENT", 22), H / 2 - 62, 22, col);
    } else {
        ui::Text(name, W - 36 - ui::Measure(name, 80), H / 2 - 110, 80, col);
    }

    // vertical progress bar toward next rank
    float lo = RANKS[rank].threshold;
    float hi = (rank < NUM_RANKS - 1) ? RANKS[rank + 1].threshold : MAX_POINTS;
    float t = (points_ - lo) / (hi - lo);
    t = t < 0 ? 0 : (t > 1 ? 1 : t);
    const int barH = 150, barX = W - 22, barY = H / 2 - 100;
    DrawRectangle(barX, barY, 8, barH, { 40, 20, 22, 255 });
    DrawRectangle(barX, barY + (int)(barH * (1.0f - t)), 8, (int)(barH * t), col);
    DrawRectangleLines(barX - 1, barY - 1, 10, barH + 2, { 90, 40, 44, 255 });

    // event popups under the rank
    for (size_t i = 0; i < popups_.size(); i++) {
        float a = popups_[i].t / POPUP_LIFE;
        unsigned char alpha = (unsigned char)(255 * (a > 1 ? 1 : a));
        Color pc = { 255, 230, 0, alpha };
        if (i == 0) pc = { 235, 230, 230, alpha };
        const char* t = popups_[i].text.c_str();
        ui::Text(t, W - 36 - ui::Measure(t, 14), H / 2 + 10 + (int)i * 20, 14, pc);
    }
}
