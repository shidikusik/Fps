#pragma once

#include "engine_config.h"

// Global tuning constants. Movement values are the heart of the game —
// tune them here, everything reads from this file.
namespace cfg {

// render constants live in the engine
// (RENDER_W / RENDER_H / MAX_DT come from engine_config.h)
inline constexpr float BASE_FOV = 95.0f;
inline constexpr float MAX_FOV_KICK = 12.0f;   // extra FOV at high speed

// --- player body ---
inline constexpr float PLAYER_HALF_W   = 0.40f;  // AABB half extents (x,z)
inline constexpr float PLAYER_HEIGHT   = 1.80f;
inline constexpr float PLAYER_CROUCH_H = 0.95f;
inline constexpr float EYE_OFFSET      = 0.15f;  // eyes this far below top of head

// --- core movement ---
inline constexpr float GRAVITY        = 34.0f;
inline constexpr float WALK_SPEED     = 12.0f;
inline constexpr float GROUND_ACCEL   = 12.0f;   // multiplier of wishspeed
inline constexpr float GROUND_FRICTION= 9.0f;
inline constexpr float JUMP_VELOCITY  = 12.5f;
inline constexpr float JUMP_BUFFER    = 0.15f;   // press jump slightly before landing
inline constexpr float COYOTE_TIME    = 0.10f;

// --- bunny hop / air control (quake style) ---
inline constexpr float AIR_WISHSPEED_CAP = 1.7f;  // strafe gain per accel tick
inline constexpr float AIR_ACCEL         = 130.0f;

// --- dash ---
inline constexpr int   DASH_CHARGES   = 3;
inline constexpr float DASH_RECHARGE  = 1.6f;    // seconds per charge
inline constexpr float DASH_SPEED     = 38.0f;
inline constexpr float DASH_TIME      = 0.13f;   // duration of the burst
inline constexpr float DASH_END_KEEP  = 16.0f;   // speed kept after a dash ends on its own

// --- slide ---
inline constexpr float SLIDE_BOOST    = 6.0f;    // added on slide start
inline constexpr float SLIDE_MIN_SPEED= 17.0f;   // slide starts at least this fast
inline constexpr float SLIDE_FRICTION = 0.9f;    // very low friction while sliding
inline constexpr float SLIDE_STEER    = 3.0f;    // sideways steering accel while sliding

// --- ground slam ---
inline constexpr float SLAM_SPEED       = 55.0f;
inline constexpr float SLAM_JUMP_WINDOW = 0.25f; // jump within this after landing...
inline constexpr float SLAM_JUMP_MULT   = 1.45f; // ...for a boosted "storage" jump

// --- misc ---
inline constexpr float KILL_PLANE   = -40.0f;    // fell off: respawn
inline constexpr float MOUSE_SENS   = 0.09f;     // degrees per pixel

} // namespace cfg
