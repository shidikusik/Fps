#include "weapons.h"
#include "arena.h"
#include "config.h"
#include "enemies.h"
#include "particles.h"
#include "player.h"
#include "sounds.h"
#include "style_meter.h"
#include "raymath.h"
#include "rlgl.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace {

// --- weapon stats ---
constexpr float REVOLVER_DMG = 34, REVOLVER_CD = 0.32f, REVOLVER_HEAL = 10;
constexpr float CHARGED_DMG = 110, CHARGED_CD = 1.1f, CHARGE_TIME = 0.6f, CHARGED_HEAL = 25;
constexpr int   PELLETS = 10;
constexpr float PELLET_DMG = 9, PELLET_SPEED = 42, PELLET_LIFE = 0.9f;
constexpr float SHOTGUN_CD = 0.75f, PELLET_HEAL = 3, PELLET_RADIUS = 0.14f;
constexpr float HEAL_RANGE = 10.0f;     // blood heals only up close
constexpr float PARRY_POINTS = 25;

constexpr Color GUN_DARK   = { 38, 34, 38, 255 };
constexpr Color GUN_BLACK  = { 16, 12, 14, 255 };
constexpr Color GUN_RED    = { 150, 18, 26, 255 };
constexpr Color GUN_YELLOW = { 255, 230, 0, 255 };

float Rnd2() { return (float)rand() / (float)RAND_MAX * 2.0f - 1.0f; }

Color Mix(Color a, Color b, float t) {
    return { (unsigned char)(a.r + (b.r - a.r) * t),
             (unsigned char)(a.g + (b.g - a.g) * t),
             (unsigned char)(a.b + (b.b - a.b) * t), 255 };
}

struct HitScan {
    float wallDist = 1e9f;
    Vector3 wallPoint{};
};

HitScan ScanWalls(Ray ray, const Arena& arena) {
    HitScan h;
    for (const Block& blk : arena.Blocks()) {
        RayCollision rc = GetRayCollisionBox(ray, blk.box);
        if (rc.hit && rc.distance > 0.01f && rc.distance < h.wallDist) {
            h.wallDist = rc.distance;
            h.wallPoint = rc.point;
        }
    }
    return h;
}

} // namespace

void Weapons::Reset() {
    current = WeaponType::Revolver;
    cd_ = charge_ = recoil_ = muzzle_ = hitMarker_ = bobT_ = 0;
    charging_ = chargeReady_ = false;
    switchT_ = 1;
    beams_.clear();
    pellets_.clear();
}

Vector3 Weapons::MuzzleWorld(const Player& pl) const {
    Vector3 eye = pl.EyePos();
    Vector3 fwd = pl.Forward();
    Vector3 right = Vector3Normalize(Vector3CrossProduct(fwd, { 0, 1, 0 }));
    Vector3 up = Vector3CrossProduct(right, fwd);
    Vector3 p = Vector3Add(eye, Vector3Scale(fwd, 0.9f));
    p = Vector3Add(p, Vector3Scale(right, 0.28f));
    p = Vector3Add(p, Vector3Scale(up, -0.22f));
    return p;
}

void Weapons::OnKill(Player& pl, StyleMeter& style, int weaponId) {
    style.AddEvent("KILL", 40);
    if (!pl.grounded) style.AddEvent("AIRSHOT!", 30);
    if (pl.timeSinceDash < 0.6f) style.AddEvent("DASHKILL!", 40);
    style.RegisterKillWeapon(weaponId);
    style.score += (long)(100 * style.Multiplier());
    hitMarker_ = 0.15f;
}

void Weapons::FireRevolver(Player& pl, const Arena& arena, EnemyManager& enemies,
                           ParticleSystem& fx, StyleMeter& style) {
    Vector3 eye = pl.EyePos();
    Vector3 fwd = pl.Forward();
    Ray ray{ eye, fwd };

    HitScan walls = ScanWalls(ray, arena);
    float bestDist = walls.wallDist;
    Enemy* bestEnemy = nullptr;
    EnemyShot* bestShot = nullptr;

    for (Enemy& e : enemies.All()) {
        if (!e.alive || e.spawnT < 1) continue;
        RayCollision rc = GetRayCollisionBox(ray, e.Box());
        if (rc.hit && rc.distance < bestDist) {
            bestDist = rc.distance;
            bestEnemy = &e;
        }
    }
    for (EnemyShot& s : enemies.Shots()) {
        if (!s.alive) continue;
        RayCollision rc = GetRayCollisionSphere(ray, s.pos, 0.45f); // generous parry box
        if (rc.hit && rc.distance < bestDist) {
            bestDist = rc.distance;
            bestShot = &s;
            bestEnemy = nullptr;
        }
    }

    Vector3 hitPoint = Vector3Add(eye, Vector3Scale(fwd, bestDist));
    if (bestShot) {
        bestShot->alive = false;
        fx.Sparks(bestShot->pos, 14);
        style.AddEvent("PARRY!", PARRY_POINTS);
        style.score += 50;
        sfx::Play(sfx::PARRY);
        hitMarker_ = 0.15f;
    } else if (bestEnemy) {
        bool killed = enemies.Damage(*bestEnemy, REVOLVER_DMG, hitPoint, fwd, fx);
        if (bestDist < HEAL_RANGE) pl.Heal(REVOLVER_HEAL);
        if (killed) OnKill(pl, style, 0);
        else hitMarker_ = 0.1f;
    } else if (walls.wallDist < 1e8f) {
        fx.Sparks(walls.wallPoint, 8);
    }

    beams_.push_back({ MuzzleWorld(pl), hitPoint, 0.07f, 0.07f,
                       { 255, 230, 120, 255 }, 0.025f });
    cd_ = REVOLVER_CD;
    recoil_ = 1.0f;
    muzzle_ = 1.0f;
    pl.AddTrauma(0.12f);
    sfx::Play(sfx::REVOLVER);
}

void Weapons::FireCharged(Player& pl, const Arena& arena, EnemyManager& enemies,
                          ParticleSystem& fx, StyleMeter& style) {
    Vector3 eye = pl.EyePos();
    Vector3 fwd = pl.Forward();
    Ray ray{ eye, fwd };

    HitScan walls = ScanWalls(ray, arena);
    // pierces everything up to the wall
    for (Enemy& e : enemies.All()) {
        if (!e.alive || e.spawnT < 1) continue;
        RayCollision rc = GetRayCollisionBox(ray, e.Box());
        if (rc.hit && rc.distance < walls.wallDist) {
            bool killed = enemies.Damage(e, CHARGED_DMG, rc.point, fwd, fx);
            if (rc.distance < HEAL_RANGE) pl.Heal(CHARGED_HEAL);
            if (killed) OnKill(pl, style, 0);
        }
    }
    for (EnemyShot& s : enemies.Shots()) {
        if (!s.alive) continue;
        RayCollision rc = GetRayCollisionSphere(ray, s.pos, 0.5f);
        if (rc.hit && rc.distance < walls.wallDist) {
            s.alive = false;
            fx.Sparks(s.pos, 10);
            style.AddEvent("PARRY!", PARRY_POINTS);
        }
    }
    Vector3 endPoint = walls.wallDist < 1e8f
        ? walls.wallPoint : Vector3Add(eye, Vector3Scale(fwd, 200));
    fx.Sparks(endPoint, 16);

    beams_.push_back({ MuzzleWorld(pl), endPoint, 0.22f, 0.22f,
                       { 230, 30, 40, 255 }, 0.09f });
    cd_ = CHARGED_CD;
    recoil_ = 1.6f;
    muzzle_ = 1.0f;
    pl.AddTrauma(0.35f);
    sfx::Play(sfx::CHARGED);
}

void Weapons::FireShotgun(Player& pl, ParticleSystem& fx) {
    Vector3 eye = pl.EyePos();
    Vector3 fwd = pl.Forward();
    Vector3 right = Vector3Normalize(Vector3CrossProduct(fwd, { 0, 1, 0 }));
    Vector3 up = Vector3CrossProduct(right, fwd);
    Vector3 muzzle = MuzzleWorld(pl);

    for (int i = 0; i < PELLETS; i++) {
        Vector3 d = fwd;
        d = Vector3Add(d, Vector3Scale(right, Rnd2() * 0.085f));
        d = Vector3Add(d, Vector3Scale(up, Rnd2() * 0.085f));
        d = Vector3Normalize(d);
        pellets_.push_back({ muzzle, Vector3Scale(d, PELLET_SPEED * (0.9f + 0.2f * Rnd2())),
                             PELLET_LIFE, true });
    }
    fx.Puff(muzzle, { 255, 160, 20, 255 }, 6, 3.0f, 0.07f, 0.15f);
    cd_ = SHOTGUN_CD;
    recoil_ = 1.4f;
    muzzle_ = 1.0f;
    pl.AddTrauma(0.22f);
    sfx::Play(sfx::SHOTGUN);
}

void Weapons::UpdatePellets(Player& pl, const Arena& arena, EnemyManager& enemies,
                            ParticleSystem& fx, StyleMeter& style, float dt) {
    for (Pellet& p : pellets_) {
        if (!p.alive) continue;
        p.life -= dt;
        if (p.life <= 0) { p.alive = false; continue; }
        p.pos.x += p.vel.x * dt;
        p.pos.y += p.vel.y * dt;
        p.pos.z += p.vel.z * dt;

        for (Enemy& e : enemies.All()) {
            if (!e.alive || e.spawnT < 1) continue;
            if (CheckCollisionBoxSphere(e.Box(), p.pos, PELLET_RADIUS)) {
                Vector3 dir = Vector3Normalize(p.vel);
                bool killed = enemies.Damage(e, PELLET_DMG, p.pos, dir, fx);
                float dist = Vector3Distance(pl.pos, e.pos);
                if (dist < HEAL_RANGE) pl.Heal(PELLET_HEAL);
                if (killed) OnKill(pl, style, 1);
                else hitMarker_ = std::max(hitMarker_, 0.1f);
                p.alive = false;
                break;
            }
        }
        if (!p.alive) continue;

        for (EnemyShot& s : enemies.Shots()) {
            if (!s.alive) continue;
            if (Vector3Distance(s.pos, p.pos) < 0.5f) {
                s.alive = false;
                p.alive = false;
                fx.Sparks(s.pos, 10);
                style.AddEvent("PARRY!", 15);
                sfx::Play(sfx::PARRY, 0.7f);
                break;
            }
        }
        if (!p.alive) continue;

        for (const Block& blk : arena.Blocks()) {
            if (CheckCollisionBoxSphere(blk.box, p.pos, PELLET_RADIUS)) {
                p.alive = false;
                fx.Sparks(p.pos, 3);
                break;
            }
        }
    }
    pellets_.erase(std::remove_if(pellets_.begin(), pellets_.end(),
                                  [](const Pellet& p) { return !p.alive; }),
                   pellets_.end());
}

void Weapons::Update(Player& pl, const Arena& arena, EnemyManager& enemies,
                     ParticleSystem& fx, StyleMeter& style, float dt) {
    cd_ = std::max(0.0f, cd_ - dt);
    recoil_ = std::max(0.0f, recoil_ - dt * 6);
    muzzle_ = std::max(0.0f, muzzle_ - dt * 14);
    hitMarker_ = std::max(0.0f, hitMarker_ - dt);
    switchT_ = std::min(1.0f, switchT_ + dt * 4);
    if (pl.grounded) bobT_ += dt * pl.HorizontalSpeed();

    for (auto& b : beams_) b.t -= dt;
    beams_.erase(std::remove_if(beams_.begin(), beams_.end(),
                                [](const Beam& b) { return b.t <= 0; }),
                 beams_.end());

    // switching
    WeaponType want = current;
    if (IsKeyPressed(KEY_ONE)) want = WeaponType::Revolver;
    if (IsKeyPressed(KEY_TWO)) want = WeaponType::Shotgun;
    float wheel = GetMouseWheelMove();
    if (wheel != 0)
        want = current == WeaponType::Revolver ? WeaponType::Shotgun
                                               : WeaponType::Revolver;
    if (want != current) {
        current = want;
        switchT_ = 0;
        charging_ = false;
        charge_ = 0;
        sfx::Play(sfx::CLICK, 0.6f);
    }
    if (switchT_ < 0.4f) return; // still raising the gun

    // revolver alt-fire: hold RMB to charge, release to fire the piercing shot
    if (current == WeaponType::Revolver) {
        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT) && cd_ <= 0) {
            charging_ = true;
            float prev = charge_;
            charge_ = std::min(1.0f, charge_ + dt / CHARGE_TIME);
            if (prev < 1.0f && charge_ >= 1.0f) sfx::Play(sfx::CLICK);
        } else if (charging_) {
            if (charge_ >= 1.0f)
                FireCharged(pl, arena, enemies, fx, style);
            charging_ = false;
            charge_ = 0;
        }
    }

    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && cd_ <= 0 && !charging_) {
        if (current == WeaponType::Revolver)
            FireRevolver(pl, arena, enemies, fx, style);
        else
            FireShotgun(pl, fx);
    }

    UpdatePellets(pl, arena, enemies, fx, style, dt);
}

void Weapons::Draw3D() const {
    for (const auto& b : beams_) {
        float a = b.t / b.maxT;
        Color c = b.color;
        c.a = (unsigned char)(255 * a);
        DrawCylinderEx(b.a, b.b, b.radius * a, b.radius * a * 0.5f, 6, c);
    }
    for (const auto& p : pellets_) {
        if (!p.alive) continue;
        DrawSphere(p.pos, 0.09f, { 255, 200, 60, 255 });
    }
}

void Weapons::DrawViewmodel(const Player& pl) const {
    Vector3 eye = pl.EyePos();

    rlPushMatrix();
    rlTranslatef(eye.x, eye.y, eye.z);
    rlRotatef(-pl.yaw, 0, 1, 0);
    rlRotatef(pl.pitch, 1, 0, 0);

    // local space: -Z is forward, +X right, +Y up
    float raise = (1.0f - switchT_) * -0.45f; // gun rises after switch
    float bobY = sinf(bobT_ * 0.55f) * 0.008f * Clamp(pl.HorizontalSpeed() / 12.0f, 0.0f, 2.0f);
    rlTranslatef(0.30f, -0.30f + raise + bobY, -0.55f);
    rlTranslatef(0, recoil_ * 0.03f, recoil_ * 0.13f);
    rlRotatef(recoil_ * 14, 1, 0, 0);

    if (current == WeaponType::Revolver) {
        Color barrelCol = GUN_DARK;
        if (charging_) barrelCol = Mix(GUN_DARK, { 230, 30, 40, 255 }, charge_);
        DrawCube({ 0, 0.02f, -0.34f }, 0.055f, 0.075f, 0.42f, barrelCol);   // barrel
        DrawCube({ 0, 0.075f, -0.50f }, 0.02f, 0.035f, 0.03f, GUN_YELLOW);  // sight
        DrawCube({ 0, 0.0f, -0.10f }, 0.08f, 0.115f, 0.18f, GUN_BLACK);     // cylinder
        DrawCube({ 0, -0.10f, 0.02f }, 0.06f, 0.16f, 0.09f, GUN_RED);       // grip
        DrawCube({ 0, -0.10f, 0.02f }, 0.075f, 0.06f, 0.105f, GUN_BLACK);   // hand
    } else {
        DrawCube({ -0.025f, 0.02f, -0.42f }, 0.05f, 0.06f, 0.62f, GUN_DARK); // barrels
        DrawCube({ 0.025f, 0.02f, -0.42f }, 0.05f, 0.06f, 0.62f, GUN_DARK);
        DrawCube({ 0, 0.02f, -0.70f }, 0.115f, 0.075f, 0.06f, GUN_YELLOW);   // muzzle band
        DrawCube({ 0, -0.015f, -0.05f }, 0.11f, 0.13f, 0.3f, GUN_BLACK);     // receiver
        DrawCube({ 0, -0.11f, 0.10f }, 0.07f, 0.15f, 0.12f, GUN_RED);        // grip
        DrawCube({ 0, -0.045f, -0.38f }, 0.09f, 0.07f, 0.14f, GUN_RED);      // pump
        DrawCube({ 0, -0.045f, -0.38f }, 0.105f, 0.055f, 0.08f, GUN_BLACK);  // fore hand
        DrawCube({ 0, -0.11f, 0.10f }, 0.085f, 0.06f, 0.135f, GUN_BLACK);    // rear hand
    }

    if (muzzle_ > 0) {
        float m = muzzle_;
        Vector3 mp = current == WeaponType::Revolver
            ? Vector3{ 0, 0.02f, -0.58f } : Vector3{ 0, 0.02f, -0.78f };
        DrawSphere(mp, 0.05f + 0.06f * m, { 255, 230, 0, 255 });
        DrawSphere(mp, 0.03f + 0.03f * m, { 255, 255, 200, 255 });
    }
    rlPopMatrix();
}

void Weapons::DrawHUD() const {
    const int W = cfg::RENDER_W, H = cfg::RENDER_H;

    // weapon name bottom right
    const char* name = current == WeaponType::Revolver ? "REVOLVER [1]" : "SHOTGUN [2]";
    DrawText(name, W - MeasureText(name, 20) - 16, H - 40, 20, { 235, 230, 230, 255 });
    // cooldown ticker
    if (cd_ > 0)
        DrawRectangle(W - 116, H - 16, (int)(100 * (1.0f - cd_ / SHOTGUN_CD)), 4,
                      { 120, 60, 64, 255 });

    // charge ring around the crosshair
    if (charging_) {
        Color c = charge_ >= 1.0f ? Color{ 230, 30, 40, 255 } : Color{ 255, 230, 0, 255 };
        DrawRing({ (float)W / 2, (float)H / 2 }, 10, 13, -90, -90 + 360 * charge_, 24, c);
    }

    // hitmarker
    if (hitMarker_ > 0) {
        Color c = { 255, 230, 0, 255 };
        DrawLine(W / 2 - 9, H / 2 - 9, W / 2 - 4, H / 2 - 4, c);
        DrawLine(W / 2 + 9, H / 2 - 9, W / 2 + 4, H / 2 - 4, c);
        DrawLine(W / 2 - 9, H / 2 + 9, W / 2 - 4, H / 2 + 4, c);
        DrawLine(W / 2 + 9, H / 2 + 9, W / 2 + 4, H / 2 + 4, c);
    }
}
