#include "enemies.h"
#include "arena.h"
#include "config.h"
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

// --- enemy stats ---
constexpr float HUSK_HP = 60,  HUSK_SPEED = 4.6f,  HUSK_DMG = 15, HUSK_RANGE = 2.3f;
constexpr float SHOOTER_HP = 40, SHOOTER_DMG = 12, SHOT_SPEED = 15.0f;
constexpr float BERSERKER_HP = 110, BERSERKER_SPEED = 8.6f, BERSERKER_DMG = 25;
constexpr float WARDEN_HP = 900, WARDEN_SPEED = 7.0f, WARDEN_DMG = 35;

constexpr float SLAM_RADIUS = 6.5f, SLAM_DMG = 65.0f;
constexpr float SHOT_RADIUS = 0.28f;

float Rnd() { return (float)rand() / (float)RAND_MAX; }
float Rnd2() { return Rnd() * 2.0f - 1.0f; }

Color Mix(Color a, Color b, float t) {
    return { (unsigned char)(a.r + (b.r - a.r) * t),
             (unsigned char)(a.g + (b.g - a.g) * t),
             (unsigned char)(a.b + (b.b - a.b) * t), 255 };
}

BoundingBox PlayerBox(const Player& pl) {
    return { { pl.pos.x - 0.4f, pl.pos.y, pl.pos.z - 0.4f },
             { pl.pos.x + 0.4f, pl.pos.y + 1.8f, pl.pos.z + 0.4f } };
}

} // namespace

Vector3 Enemy::HalfSize() const {
    switch (type) {
        case EnemyType::Husk:      return { 0.50f, 1.05f, 0.50f };
        case EnemyType::Shooter:   return { 0.60f, 0.60f, 0.60f };
        case EnemyType::Berserker: return { 0.75f, 0.95f, 0.75f };
        case EnemyType::Warden:    return { 1.60f, 2.10f, 1.60f };
    }
    return { 0.5f, 0.5f, 0.5f };
}

Vector3 Enemy::Center() const {
    return { pos.x, pos.y + HalfSize().y, pos.z };
}

BoundingBox Enemy::Box() const {
    Vector3 h = HalfSize();
    return { { pos.x - h.x, pos.y, pos.z - h.z },
             { pos.x + h.x, pos.y + h.y * 2, pos.z + h.z } };
}

void EnemyManager::BeginLevel(int lvl, int lp) {
    enemies_.clear();
    shots_.clear();
    level = lvl;
    loop = lp;
    waveInLevel = 0;
    waveActive = false;
    levelCleared = false;
    intermission = 3.0f;
}

int EnemyManager::AliveCount() const {
    int n = 0;
    for (const Enemy& e : enemies_) n += e.alive ? 1 : 0;
    return n;
}

const Enemy* EnemyManager::Boss() const {
    for (const Enemy& e : enemies_)
        if (e.alive && e.type == EnemyType::Warden && e.spawnDelay <= 0) return &e;
    return nullptr;
}

void EnemyManager::Spawn(EnemyType t, Vector3 pos, float delay) {
    Enemy e;
    e.type = t;
    e.pos = pos;
    e.spawnDelay = delay;
    e.strafeSign = Rnd() > 0.5f ? 1.0f : -1.0f;
    float s = DiffScale();
    switch (t) {
        case EnemyType::Husk:      e.hp = e.maxHp = HUSK_HP * s; break;
        case EnemyType::Shooter:   e.hp = e.maxHp = SHOOTER_HP * s; e.pos.y += 3; break;
        case EnemyType::Berserker: e.hp = e.maxHp = BERSERKER_HP * s; break;
        case EnemyType::Warden:    e.hp = e.maxHp = WARDEN_HP * s; break;
    }
    enemies_.push_back(e);
}

void EnemyManager::StartWave(int w, const Arena& arena) {
    waveInLevel = w;
    totalWaves++;
    waveActive = true;
    enemies_.erase(std::remove_if(enemies_.begin(), enemies_.end(),
                                  [](const Enemy& e) { return !e.alive; }),
                   enemies_.end());

    // composition scales with level, wave and NG+ loop
    int husks = 0, shooters = 0, berserkers = 0;
    bool boss = false;
    switch (level) {
        case 1:
            husks = 3 + w * 2;
            shooters = w - 1;
            berserkers = w >= 4 ? 1 : 0;
            break;
        case 2:
            husks = 4 + w * 2;
            shooters = w;
            berserkers = (w + 1) / 2;
            break;
        case 3:
            husks = 5 + w * 2;
            shooters = w + 1;
            berserkers = w;
            break;
        case 4:
            husks = 6 + w * 2;
            shooters = w + 1;
            berserkers = w + 1;
            break;
        default: // 5: the Warden's floor
            if (w == WAVES_PER_LEVEL) {
                boss = true;
                husks = 4;
                shooters = 2;
            } else {
                husks = 6 + w * 2;
                shooters = w + 2;
                berserkers = w + 1;
            }
            break;
    }
    husks = std::min(husks + loop, 16);
    shooters = std::min(shooters + (loop > 0 ? 1 : 0), 7);
    berserkers = std::min(berserkers + (loop > 0 ? 1 : 0), 6);

    const std::vector<Vector3>& pads = arena.SpawnPads();
    int idx = rand() % (int)pads.size();
    float delay = 0;
    auto pad = [&]() {
        Vector3 p = pads[idx++ % pads.size()];
        p.x += Rnd2() * 2.5f;
        p.z += Rnd2() * 2.5f;
        return p;
    };
    for (int i = 0; i < husks; i++)      { Spawn(EnemyType::Husk, pad(), delay); delay += 0.35f; }
    for (int i = 0; i < shooters; i++)   { Spawn(EnemyType::Shooter, pad(), delay); delay += 0.45f; }
    for (int i = 0; i < berserkers; i++) { Spawn(EnemyType::Berserker, pad(), delay); delay += 0.6f; }
    if (boss) Spawn(EnemyType::Warden, arena.BossPad(), 1.0f); // on the throne

    sfx::Play(sfx::WAVE, boss ? 1.0f : 0.9f, boss ? 0.7f : 1.0f);
}

bool EnemyManager::Damage(Enemy& e, float dmg, Vector3 hitPoint, Vector3 dir,
                          ParticleSystem& fx) {
    if (!e.alive || e.spawnT < 1.0f) return false;
    e.hp -= dmg;
    e.hitFlash = 1.0f;
    fx.Blood(hitPoint, dir, 10);
    if (e.hp <= 0) {
        e.alive = false;
        fx.BigBlood(e.Center(), e.type == EnemyType::Warden ? 120 : 40);
        sfx::Play(sfx::KILL, 1.0f, e.type == EnemyType::Warden ? 0.6f : 1.0f);
        return true;
    }
    sfx::Play(sfx::HIT, 0.8f);
    return false;
}

void EnemyManager::MoveEnemy(Enemy& e, const Arena& arena, float dt) {
    Vector3 h = e.HalfSize();
    auto overlaps = [&](const BoundingBox& b) {
        return e.pos.x + h.x > b.min.x && e.pos.x - h.x < b.max.x &&
               e.pos.y + h.y * 2 > b.min.y && e.pos.y < b.max.y &&
               e.pos.z + h.z > b.min.z && e.pos.z - h.z < b.max.z;
    };

    e.pos.x += e.vel.x * dt;
    for (const Block& blk : arena.Blocks()) {
        if (!overlaps(blk.box)) continue;
        float c = (blk.box.min.x + blk.box.max.x) * 0.5f;
        e.pos.x = (e.pos.x < c) ? blk.box.min.x - h.x : blk.box.max.x + h.x;
        e.vel.x = 0;
    }
    e.pos.z += e.vel.z * dt;
    for (const Block& blk : arena.Blocks()) {
        if (!overlaps(blk.box)) continue;
        float c = (blk.box.min.z + blk.box.max.z) * 0.5f;
        e.pos.z = (e.pos.z < c) ? blk.box.min.z - h.z : blk.box.max.z + h.z;
        e.vel.z = 0;
    }
    e.pos.y += e.vel.y * dt;
    e.grounded = false;
    for (const Block& blk : arena.Blocks()) {
        if (!overlaps(blk.box)) continue;
        float c = (blk.box.min.y + blk.box.max.y) * 0.5f;
        if (e.pos.y + h.y < c) {
            e.pos.y = blk.box.min.y - h.y * 2;
            e.vel.y = std::min(e.vel.y, 0.0f);
        } else {
            e.pos.y = blk.box.max.y;
            if (e.vel.y <= 0) { e.vel.y = 0; e.grounded = true; }
        }
    }
}

void EnemyManager::UpdateHusk(Enemy& e, Player& pl, float dist, Vector3 dir, float dt) {
    e.vel.x += (dir.x * HUSK_SPEED - e.vel.x) * std::min(1.0f, dt * 6);
    e.vel.z += (dir.z * HUSK_SPEED - e.vel.z) * std::min(1.0f, dt * 6);
    e.animT += dt * Vector2Length({ e.vel.x, e.vel.z });
    if (dist < HUSK_RANGE && e.attackCd <= 0) {
        e.attackCd = 1.3f;
        pl.TakeDamage(HUSK_DMG);
        e.vel.x += dir.x * 5; // lunge
        e.vel.z += dir.z * 5;
    }
}

void EnemyManager::UpdateShooter(Enemy& e, Player& pl, const Arena& arena,
                                 float dist, Vector3 dir, float dt) {
    // hold a mid-range ring around the player, strafing sideways
    float radial = 0;
    if (dist > 20) radial = 1;
    else if (dist < 12) radial = -1;
    Vector3 tangent = { -dir.z * e.strafeSign, 0, dir.x * e.strafeSign };
    e.vel.x += ((dir.x * radial + tangent.x * 0.7f) * 5.0f - e.vel.x) * std::min(1.0f, dt * 4);
    e.vel.z += ((dir.z * radial + tangent.z * 0.7f) * 5.0f - e.vel.z) * std::min(1.0f, dt * 4);

    // hover: float toward player's eye level + bob
    e.animT += dt;
    float targetY = Clamp(pl.EyePos().y + 1.5f, 2.5f, 12.0f) + sinf(e.animT * 2.2f) * 0.4f;
    e.vel.y = (targetY - e.pos.y) * 2.0f;

    e.cd2 -= dt;
    if (e.cd2 <= 0) { e.strafeSign = -e.strafeSign; e.cd2 = 2.0f + Rnd() * 2.5f; }

    if (e.attackCd <= 0 && dist < 38) {
        Vector3 from = e.Center();
        Vector3 to = pl.EyePos();
        Vector3 d = Vector3Subtract(to, from);
        float playerDist = Vector3Length(d);
        Ray ray{ from, Vector3Scale(d, 1.0f / playerDist) };
        bool blocked = false;
        for (const Block& blk : arena.Blocks()) {
            RayCollision rc = GetRayCollisionBox(ray, blk.box);
            if (rc.hit && rc.distance < playerDist) { blocked = true; break; }
        }
        if (!blocked) {
            e.attackCd = 2.1f + Rnd() * 0.7f;
            Vector3 aim = Vector3Add(to, Vector3Scale(pl.vel, playerDist / SHOT_SPEED * 0.35f));
            Vector3 sd = Vector3Normalize(Vector3Subtract(aim, from));
            shots_.push_back({ from, Vector3Scale(sd, SHOT_SPEED), 6.0f, true });
            sfx::Play(sfx::ENEMY_SHOOT, 0.7f);
        }
    }
}

void EnemyManager::UpdateBerserker(Enemy& e, Player& pl, float dist, Vector3 dir, float dt) {
    e.cd2 -= dt;
    if (e.grounded) {
        e.vel.x += (dir.x * BERSERKER_SPEED - e.vel.x) * std::min(1.0f, dt * 8);
        e.vel.z += (dir.z * BERSERKER_SPEED - e.vel.z) * std::min(1.0f, dt * 8);
        if (dist > 5 && dist < 11 && e.cd2 <= 0) { // pounce
            e.cd2 = 2.8f;
            e.vel = { dir.x * 13, 10.5f, dir.z * 13 };
        }
    }
    e.animT += dt * Vector2Length({ e.vel.x, e.vel.z });
    if (dist < 2.7f && e.attackCd <= 0) {
        e.attackCd = 1.2f;
        pl.TakeDamage(BERSERKER_DMG);
    }
}

void EnemyManager::UpdateWarden(Enemy& e, Player& pl, ParticleSystem& fx,
                                float dist, Vector3 dir, float dt) {
    e.cd2 -= dt;
    if (e.grounded) {
        e.vel.x += (dir.x * WARDEN_SPEED - e.vel.x) * std::min(1.0f, dt * 5);
        e.vel.z += (dir.z * WARDEN_SPEED - e.vel.z) * std::min(1.0f, dt * 5);
        if (dist > 7 && dist < 20 && e.cd2 <= 0) { // long pounce
            e.cd2 = 3.5f;
            e.vel = { dir.x * 15, 12.0f, dir.z * 15 };
        }
    }
    // landing shockwave: a ring of destructible shots
    if (e.wasAir && e.grounded) {
        fx.Puff(e.pos, { 255, 160, 20, 255 }, 30, 10.0f, 0.18f, 0.5f);
        sfx::Play(sfx::SLAM, 0.9f, 0.8f);
        Vector3 c = e.Center();
        for (int i = 0; i < 10; i++) {
            float a = (float)i / 10.0f * 6.2831853f;
            Vector3 v = { cosf(a) * 11.0f, 0.5f, sinf(a) * 11.0f };
            shots_.push_back({ { c.x, e.pos.y + 1.0f, c.z }, v, 3.0f, true });
        }
    }
    e.wasAir = !e.grounded;
    e.animT += dt * Vector2Length({ e.vel.x, e.vel.z });
    if (dist < 4.0f && e.attackCd <= 0) {
        e.attackCd = 1.5f;
        pl.TakeDamage(WARDEN_DMG);
    }
}

void EnemyManager::Update(Player& pl, const Arena& arena, ParticleSystem& fx,
                          StyleMeter& style, float dt) {
    // --- wave state machine (levelCleared is consumed by main) ---
    if (!waveActive && !levelCleared) {
        intermission -= dt;
        if (intermission <= 0) StartWave(waveInLevel + 1, arena);
    } else if (waveActive && AliveCount() == 0) {
        waveActive = false;
        style.score += 250L * (waveInLevel + (level - 1) * WAVES_PER_LEVEL);
        style.AddEvent("WAVE CLEAR", 60);
        if (waveInLevel >= WAVES_PER_LEVEL) levelCleared = true;
        else intermission = 4.0f;
    }

    // --- ground slam AoE ---
    if (pl.slamLandedThisFrame) {
        fx.Puff({ pl.pos.x, pl.pos.y + 0.3f, pl.pos.z }, { 255, 160, 20, 255 },
                24, 9.0f, 0.14f, 0.4f);
        for (Enemy& e : enemies_) {
            if (!e.alive || e.spawnT < 1) continue;
            float dx = e.pos.x - pl.pos.x, dz = e.pos.z - pl.pos.z;
            float d = sqrtf(dx * dx + dz * dz);
            if (d < SLAM_RADIUS && fabsf(e.pos.y - pl.pos.y) < 4.0f) {
                bool killed = Damage(e, SLAM_DMG, e.Center(), { 0, 1, 0 }, fx);
                if (killed) {
                    style.AddEvent("SLAMDUNK!", 45);
                    style.score += (long)(100 * style.Multiplier());
                    pl.Heal(15);
                } else if (e.type != EnemyType::Warden) {
                    e.vel.y += 8; // survivors get launched (not the boss)
                }
            }
        }
    }

    // --- enemies ---
    for (Enemy& e : enemies_) {
        if (!e.alive) continue;
        if (e.spawnDelay > 0) { e.spawnDelay -= dt; continue; }
        if (e.spawnT < 1.0f) {
            if (e.spawnT == 0)
                fx.Puff(e.Center(), { 230, 30, 40, 255 }, 14, 4.0f, 0.12f, 0.5f);
            e.spawnT = std::min(1.0f, e.spawnT + dt / 0.6f);
            continue;
        }

        e.hitFlash = std::max(0.0f, e.hitFlash - dt * 6);
        e.attackCd = std::max(0.0f, e.attackCd - dt);

        Vector3 to = { pl.pos.x - e.pos.x, 0, pl.pos.z - e.pos.z };
        Vector3 plCenter = { pl.pos.x, pl.pos.y + 0.9f, pl.pos.z };
        float dist = Vector3Length(Vector3Subtract(plCenter, e.Center()));
        float flat = Vector2Length({ to.x, to.z });
        Vector3 dir = flat > 0.01f ? Vector3Scale(to, 1.0f / flat) : Vector3{ 0, 0, 1 };
        e.faceDir = dir;

        switch (e.type) {
            case EnemyType::Husk:      UpdateHusk(e, pl, dist, dir, dt); break;
            case EnemyType::Shooter:   UpdateShooter(e, pl, arena, dist, dir, dt); break;
            case EnemyType::Berserker: UpdateBerserker(e, pl, dist, dir, dt); break;
            case EnemyType::Warden:    UpdateWarden(e, pl, fx, dist, dir, dt); break;
        }

        if (e.type != EnemyType::Shooter) e.vel.y -= cfg::GRAVITY * dt;
        MoveEnemy(e, arena, dt);
        if (e.pos.y < cfg::KILL_PLANE) e.alive = false;
    }

    // --- cheap pairwise separation ---
    for (size_t i = 0; i < enemies_.size(); i++) {
        Enemy& a = enemies_[i];
        if (!a.alive || a.spawnT < 1) continue;
        for (size_t j = i + 1; j < enemies_.size(); j++) {
            Enemy& b = enemies_[j];
            if (!b.alive || b.spawnT < 1) continue;
            float dx = b.pos.x - a.pos.x, dz = b.pos.z - a.pos.z;
            float d = sqrtf(dx * dx + dz * dz);
            float want = (a.HalfSize().x + b.HalfSize().x) * 1.25f;
            if (d > 0.01f && d < want) {
                float push = (want - d) * 0.5f / d;
                a.pos.x -= dx * push; a.pos.z -= dz * push;
                b.pos.x += dx * push; b.pos.z += dz * push;
            }
        }
    }

    // --- enemy projectiles ---
    for (EnemyShot& s : shots_) {
        if (!s.alive) continue;
        s.life -= dt;
        if (s.life <= 0) { s.alive = false; continue; }
        s.pos.x += s.vel.x * dt;
        s.pos.y += s.vel.y * dt;
        s.pos.z += s.vel.z * dt;
        for (const Block& blk : arena.Blocks()) {
            if (CheckCollisionBoxSphere(blk.box, s.pos, SHOT_RADIUS)) {
                s.alive = false;
                fx.Sparks(s.pos, 6);
                break;
            }
        }
        if (s.alive && CheckCollisionBoxSphere(PlayerBox(pl), s.pos, SHOT_RADIUS)) {
            s.alive = false;
            pl.TakeDamage(SHOOTER_DMG);
            fx.Sparks(s.pos, 8);
        }
    }
    shots_.erase(std::remove_if(shots_.begin(), shots_.end(),
                                [](const EnemyShot& s) { return !s.alive; }),
                 shots_.end());
}

void EnemyManager::Draw() const {
    for (const Enemy& e : enemies_) {
        if (!e.alive || e.spawnDelay > 0) continue;
        float sc = e.spawnT;
        Vector3 c = e.Center();
        float flash = e.hitFlash;
        float yawDeg = RAD2DEG * atan2f(e.faceDir.x, e.faceDir.z);

        rlPushMatrix();
        rlTranslatef(c.x, c.y, c.z);
        rlRotatef(yawDeg, 0, 1, 0);
        rlScalef(sc, sc, sc);

        switch (e.type) {
            case EnemyType::Husk: {
                Color body = Mix({ 120, 26, 30, 255 }, WHITE, flash);
                Color head = Mix({ 20, 14, 16, 255 }, WHITE, flash);
                float sway = sinf(e.animT * 1.6f) * 0.25f;
                DrawCube({ 0, 0.05f, 0 }, 0.9f, 1.15f, 0.55f, body);
                DrawCubeWires({ 0, 0.05f, 0 }, 0.9f, 1.15f, 0.55f, { 220, 30, 40, 255 });
                DrawCube({ 0, 0.9f, 0 }, 0.45f, 0.45f, 0.45f, head);
                DrawCube({ 0.1f, 0.92f, 0.23f }, 0.09f, 0.09f, 0.05f, { 255, 230, 0, 255 });
                DrawCube({ -0.1f, 0.92f, 0.23f }, 0.09f, 0.09f, 0.05f, { 255, 230, 0, 255 });
                DrawCube({ 0.55f, 0.15f, sway }, 0.2f, 0.8f, 0.2f, body);
                DrawCube({ -0.55f, 0.15f, -sway }, 0.2f, 0.8f, 0.2f, body);
                DrawCube({ 0.22f, -0.85f, sway * 0.7f }, 0.25f, 0.5f, 0.25f, head);
                DrawCube({ -0.22f, -0.85f, -sway * 0.7f }, 0.25f, 0.5f, 0.25f, head);
                break;
            }
            case EnemyType::Shooter: {
                Color body = Mix({ 22, 16, 18, 255 }, WHITE, flash);
                rlPushMatrix();
                rlRotatef(sinf(e.animT * 1.4f) * 8.0f, 0, 0, 1); // idle tilt
                DrawCylinder({ 0, 0, 0 }, 0.62f, 0.0f, 0.62f, 4, body);
                DrawCylinderWires({ 0, 0, 0 }, 0.62f, 0.0f, 0.62f, 4, { 230, 30, 40, 255 });
                rlPushMatrix();
                rlRotatef(180, 1, 0, 0);
                DrawCylinder({ 0, 0, 0 }, 0.62f, 0.0f, 0.62f, 4, body);
                DrawCylinderWires({ 0, 0, 0 }, 0.62f, 0.0f, 0.62f, 4, { 230, 30, 40, 255 });
                rlPopMatrix();
                rlPopMatrix();
                DrawSphere({ 0, 0, 0.45f }, 0.22f, Mix({ 255, 230, 0, 255 }, WHITE, flash));
                DrawSphere({ 0, 0, 0.58f }, 0.09f, { 20, 14, 16, 255 });
                break;
            }
            case EnemyType::Berserker: {
                Color body = Mix({ 190, 20, 26, 255 }, WHITE, flash);
                Color dark = Mix({ 30, 12, 14, 255 }, WHITE, flash);
                float lean = Clamp(Vector2Length({ e.vel.x, e.vel.z }) * 2.0f, 0.0f, 14.0f);
                rlRotatef(lean, 1, 0, 0);
                DrawCube({ 0, 0.1f, 0 }, 1.4f, 1.15f, 0.9f, body);
                DrawCubeWires({ 0, 0.1f, 0 }, 1.4f, 1.15f, 0.9f, { 255, 230, 0, 255 });
                DrawCube({ 0, 0.85f, 0.15f }, 0.55f, 0.4f, 0.5f, dark);
                DrawCube({ 0, 0.85f, 0.42f }, 0.3f, 0.08f, 0.05f, { 255, 230, 0, 255 });
                DrawCylinder({ 0.35f, 1.0f, 0 }, 0.12f, 0.0f, 0.45f, 4, dark);
                DrawCylinder({ -0.35f, 1.0f, 0 }, 0.12f, 0.0f, 0.45f, 4, dark);
                DrawCube({ 0.85f, -0.1f, 0.1f }, 0.35f, 0.9f, 0.35f, dark);
                DrawCube({ -0.85f, -0.1f, 0.1f }, 0.35f, 0.9f, 0.35f, dark);
                break;
            }
            case EnemyType::Warden: {
                Color body = Mix({ 26, 12, 16, 255 }, WHITE, flash);
                Color plate = Mix({ 120, 12, 20, 255 }, WHITE, flash);
                Color gold = Mix({ 255, 220, 120, 255 }, WHITE, flash);
                float lean = Clamp(Vector2Length({ e.vel.x, e.vel.z }) * 1.5f, 0.0f, 12.0f);
                rlRotatef(lean, 1, 0, 0);
                DrawCube({ 0, 0.1f, 0 }, 3.0f, 2.6f, 2.0f, body);
                DrawCubeWires({ 0, 0.1f, 0 }, 3.0f, 2.6f, 2.0f, { 230, 30, 40, 255 });
                DrawCube({ 0, 0.3f, 0.9f }, 2.2f, 1.4f, 0.3f, plate);      // chest plate
                DrawCube({ 0, 1.8f, 0.2f }, 1.1f, 0.9f, 1.0f, plate);      // head
                DrawCube({ 0, 1.8f, 0.75f }, 0.8f, 0.15f, 0.1f, gold);     // visor
                DrawCylinder({ 0.7f, 2.2f, 0 }, 0.22f, 0.0f, 0.9f, 4, gold);  // crown
                DrawCylinder({ -0.7f, 2.2f, 0 }, 0.22f, 0.0f, 0.9f, 4, gold);
                DrawCylinder({ 0, 2.3f, -0.3f }, 0.22f, 0.0f, 1.1f, 4, gold);
                DrawCube({ 1.8f, -0.2f, 0.2f }, 0.7f, 2.0f, 0.7f, body);   // arms
                DrawCube({ -1.8f, -0.2f, 0.2f }, 0.7f, 2.0f, 0.7f, body);
                DrawCubeWires({ 1.8f, -0.2f, 0.2f }, 0.7f, 2.0f, 0.7f, { 230, 30, 40, 255 });
                DrawCubeWires({ -1.8f, -0.2f, 0.2f }, 0.7f, 2.0f, 0.7f, { 230, 30, 40, 255 });
                break;
            }
        }
        rlPopMatrix();
    }

    for (const EnemyShot& s : shots_) {
        if (!s.alive) continue;
        DrawSphere(s.pos, SHOT_RADIUS, { 255, 230, 0, 255 });
        DrawSphereWires(s.pos, SHOT_RADIUS + 0.06f, 4, 4, { 255, 160, 20, 255 });
    }
}
