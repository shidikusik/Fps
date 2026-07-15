#pragma once

#include "raylib.h"
#include <vector>

class Arena;
class Player;
class EnemyManager;
class ParticleSystem;
class StyleMeter;

enum class WeaponType { Revolver = 0, Shotgun = 1 };

class Weapons {
public:
    void Reset();
    // Reads mouse/keyboard directly; call only while the game is live.
    void Update(Player& pl, const Arena& arena, EnemyManager& enemies,
                ParticleSystem& fx, StyleMeter& style, float dt);
    void Draw3D() const;                        // tracers, pellets, muzzle flash
    void DrawViewmodel(const Player& pl) const; // call inside its own 3D pass
    void DrawHUD() const;                       // weapon name, charge ring, hitmarker

    WeaponType current = WeaponType::Revolver;

private:
    struct Beam { Vector3 a, b; float t, maxT; Color color; float radius; };
    struct Pellet { Vector3 pos, vel; float life; bool alive; };

    void FireRevolver(Player& pl, const Arena& arena, EnemyManager& enemies,
                      ParticleSystem& fx, StyleMeter& style);
    void FireCharged(Player& pl, const Arena& arena, EnemyManager& enemies,
                     ParticleSystem& fx, StyleMeter& style);
    void FireShotgun(Player& pl, ParticleSystem& fx);
    void UpdatePellets(Player& pl, const Arena& arena, EnemyManager& enemies,
                       ParticleSystem& fx, StyleMeter& style, float dt);
    void OnKill(Player& pl, StyleMeter& style, int weaponId);
    Vector3 MuzzleWorld(const Player& pl) const;

    float cd_ = 0;
    float charge_ = 0;
    bool  charging_ = false;
    bool  chargeReady_ = false;
    float recoil_ = 0;
    float muzzle_ = 0;
    float switchT_ = 1;     // raise animation after switching
    float bobT_ = 0;
    float hitMarker_ = 0;
    std::vector<Beam> beams_;
    std::vector<Pellet> pellets_;
};
