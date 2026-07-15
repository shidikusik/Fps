#pragma once

#include <string>
#include <vector>

// ULTRAKILL-style rank meter: D -> C -> B -> A -> S -> ULTRAVIOLENT.
// Kills, variety and speed feed it; passivity drains it.
class StyleMeter {
public:
    void Reset();
    void AddEvent(const char* label, float pts); // popup + points
    void RegisterKillWeapon(int weaponId);       // awards VARIETY bonus
    void Update(float dt);
    void Draw() const;                           // right-side meter + popups

    int RankIndex() const;                       // 0..5
    float Multiplier() const { return 1.0f + RankIndex() * 0.25f; }

    long score = 0;

private:
    struct Popup { std::string text; float t; };

    float points_ = 0;
    float freshness_ = 0;       // recent action keeps the meter from draining
    int lastKillWeapon_ = -1;
    std::vector<Popup> popups_;
};
