#pragma once

#include "raylib.h"
#include <vector>

class Arena;
class Player;
class ParticleSystem;
class StyleMeter;

enum class EnemyType { Husk, Shooter, Berserker };

struct Enemy {
    EnemyType type;
    Vector3 pos{};        // feet center
    Vector3 vel{};
    Vector3 faceDir{ 0, 0, 1 };
    float hp = 1, maxHp = 1;
    float hitFlash = 0;
    float attackCd = 0;   // melee / shoot cooldown
    float cd2 = 0;        // berserker leap cooldown / shooter strafe flip
    float strafeSign = 1;
    float animT = 0;
    float spawnDelay = 0; // staggered wave spawn
    float spawnT = 0;     // 0..1 materialize
    bool grounded = false;
    bool alive = true;

    Vector3 HalfSize() const;
    Vector3 Center() const;
    BoundingBox Box() const;
};

// Destructible enemy projectile — shoot it for a parry.
struct EnemyShot {
    Vector3 pos{}, vel{};
    float life = 6;
    bool alive = true;
};

class EnemyManager {
public:
    void Reset();  // wave 1 starts after a short countdown
    void Update(Player& pl, const Arena& arena, ParticleSystem& fx,
                StyleMeter& style, float dt);
    void Draw() const;

    // Applies damage + gore; returns true if this hit killed the enemy.
    bool Damage(Enemy& e, float dmg, Vector3 hitPoint, Vector3 dir,
                ParticleSystem& fx);

    std::vector<Enemy>& All() { return enemies_; }
    std::vector<EnemyShot>& Shots() { return shots_; }
    int AliveCount() const;

    int wave = 0;
    bool waveActive = false;
    float intermission = 0; // countdown to next wave when !waveActive

private:
    void StartWave(int w);
    void Spawn(EnemyType t, Vector3 pos, float delay);
    void UpdateHusk(Enemy& e, Player& pl, float dist, Vector3 dir, float dt);
    void UpdateShooter(Enemy& e, Player& pl, const Arena& arena, float dist,
                       Vector3 dir, float dt);
    void UpdateBerserker(Enemy& e, Player& pl, float dist, Vector3 dir, float dt);
    void MoveEnemy(Enemy& e, const Arena& arena, float dt);

    std::vector<Enemy> enemies_;
    std::vector<EnemyShot> shots_;
};
