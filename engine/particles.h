#pragma once

#include "raylib.h"
#include <vector>

struct Particle {
    Vector3 pos, vel;
    float life = 0, maxLife = 1;
    float size = 0.1f;
    Color color{};
    bool gravity = true;
    bool bounce = false;
};

// Physical debris: blood cubes, sparks, puffs. Cheap, capped, no collision
// with level geometry except the main floor plane.
class ParticleSystem {
public:
    void Reset() { ps_.clear(); }
    void Blood(Vector3 pos, Vector3 dir, int count);
    void BigBlood(Vector3 pos, int count = 36);
    void Sparks(Vector3 pos, int count = 8);
    void Puff(Vector3 pos, Color color, int count, float speed, float size, float life);
    void Update(float dt);
    void Draw() const;

private:
    void Push(const Particle& p);
    std::vector<Particle> ps_;
};
