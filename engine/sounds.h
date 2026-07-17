#pragma once

// Procedurally generated sound effects — no asset files.
namespace sfx {

enum Id {
    REVOLVER, SHOTGUN, CHARGED, HIT, KILL, HURT, HEAL,
    DASH, SLAM, PARRY, ENEMY_SHOOT, WAVE, CLICK, NAIL, RAIL,
    COUNT
};

void Init();     // call after InitWindow
void Shutdown();
// pitch: 1.0 = as generated; small random detune is added on top.
void Play(Id id, float volume = 1.0f, float pitch = 1.0f);

} // namespace sfx
