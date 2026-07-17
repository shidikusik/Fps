#include "particles.h"
#include <cstdlib>

namespace {
constexpr int MAX_PARTICLES = 1400;

float Rnd() { return (float)rand() / (float)RAND_MAX; }          // 0..1
float Rnd2() { return Rnd() * 2.0f - 1.0f; }                     // -1..1
} // namespace

void ParticleSystem::Push(const Particle& p) {
    if ((int)ps_.size() < MAX_PARTICLES) ps_.push_back(p);
    else ps_[rand() % MAX_PARTICLES] = p; // full: recycle a random slot
}

void ParticleSystem::Blood(Vector3 pos, Vector3 dir, int count) {
    for (int i = 0; i < count; i++) {
        Particle p;
        p.pos = pos;
        p.vel = { dir.x * 4 + Rnd2() * 5, dir.y * 4 + Rnd() * 6, dir.z * 4 + Rnd2() * 5 };
        p.maxLife = p.life = 0.6f + Rnd() * 0.7f;
        p.size = 0.08f + Rnd() * 0.12f;
        unsigned char r = (unsigned char)(150 + Rnd() * 90);
        p.color = { r, 10, 18, 255 };
        p.gravity = true;
        p.bounce = true;
        Push(p);
    }
}

void ParticleSystem::BigBlood(Vector3 pos, int count) {
    for (int i = 0; i < count; i++) {
        Particle p;
        p.pos = pos;
        p.vel = { Rnd2() * 9, Rnd() * 10, Rnd2() * 9 };
        p.maxLife = p.life = 0.8f + Rnd() * 0.9f;
        p.size = 0.10f + Rnd() * 0.20f;
        unsigned char r = (unsigned char)(140 + Rnd() * 100);
        p.color = { r, 8, 16, 255 };
        p.gravity = true;
        p.bounce = true;
        Push(p);
    }
}

void ParticleSystem::Sparks(Vector3 pos, int count) {
    for (int i = 0; i < count; i++) {
        Particle p;
        p.pos = pos;
        p.vel = { Rnd2() * 8, Rnd() * 7, Rnd2() * 8 };
        p.maxLife = p.life = 0.15f + Rnd() * 0.3f;
        p.size = 0.05f + Rnd() * 0.05f;
        p.color = (i % 3 == 0) ? Color{ 255, 230, 0, 255 } : Color{ 255, 160, 20, 255 };
        p.gravity = false;
        Push(p);
    }
}

void ParticleSystem::Puff(Vector3 pos, Color color, int count, float speed, float size, float life) {
    for (int i = 0; i < count; i++) {
        Particle p;
        p.pos = pos;
        p.vel = { Rnd2() * speed, Rnd2() * speed, Rnd2() * speed };
        p.maxLife = p.life = life * (0.6f + Rnd() * 0.8f);
        p.size = size * (0.6f + Rnd() * 0.8f);
        p.color = color;
        p.gravity = false;
        Push(p);
    }
}

void ParticleSystem::Update(float dt) {
    for (size_t i = 0; i < ps_.size();) {
        Particle& p = ps_[i];
        p.life -= dt;
        if (p.life <= 0) {
            p = ps_.back();
            ps_.pop_back();
            continue;
        }
        if (p.gravity) p.vel.y -= 26.0f * dt;
        p.pos.x += p.vel.x * dt;
        p.pos.y += p.vel.y * dt;
        p.pos.z += p.vel.z * dt;
        // bounce off the main floor (y = 0) inside the arena footprint
        if (p.bounce && p.pos.y < p.size * 0.5f && p.vel.y < 0 &&
            p.pos.x > -50 && p.pos.x < 50 && p.pos.z > -50 && p.pos.z < 50) {
            p.pos.y = p.size * 0.5f;
            p.vel.y *= -0.35f;
            p.vel.x *= 0.6f;
            p.vel.z *= 0.6f;
        }
        i++;
    }
}

void ParticleSystem::Draw() const {
    for (const Particle& p : ps_) {
        float t = p.life / p.maxLife;
        float s = p.size * (0.4f + 0.6f * t);
        DrawCube(p.pos, s, s, s, p.color);
    }
}
