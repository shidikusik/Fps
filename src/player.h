#pragma once

#include "raylib.h"

class Arena;

// Per-frame input snapshot, gathered in main and fed to the player.
struct PlayerInput {
    float   mouseDx = 0, mouseDy = 0;
    float   fwd = 0, side = 0;      // -1..1 WASD axes
    bool    jumpPressed = false;    // edge
    bool    jumpHeld = false;       // for auto-bhop
    bool    dashPressed = false;
    bool    crouchPressed = false;
    bool    crouchHeld = false;
};

class Player {
public:
    void Init(Vector3 spawn);
    void Update(const PlayerInput& in, const Arena& arena, float dt);

    Camera3D GetCamera(float shakeOffX, float shakeOffY) const;

    // --- state read by HUD / rest of the game ---
    Vector3 pos{};                  // feet position (bottom center of AABB)
    Vector3 vel{};
    float   yaw = 0, pitch = 0;     // degrees
    bool    grounded = false;
    bool    sliding = false;
    bool    dashing = false;
    bool    slamming = false;
    float   dashCharges = (float)3; // fractional while recharging
    float   trauma = 0;             // 0..1 screenshake energy

    float HorizontalSpeed() const;
    Vector3 Forward() const;        // full look direction
    Vector3 FlatForward() const;    // yaw only, y=0

    void AddTrauma(float t);

private:
    void Accelerate(Vector3 wishdir, float wishspeed, float accel, float dt);
    void ApplyFriction(float friction, float dt);
    void MoveAndCollide(const Arena& arena, float dt);
    void StartDash(Vector3 dir);
    void StartSlide();
    void EndSlide();

    Vector3 spawn_{};
    float height_ = 1.8f;           // current hitbox height (crouch lerp)
    float jumpBuffer_ = 0;
    float coyote_ = 0;
    float dashTimeLeft_ = 0;
    Vector3 dashDir_{};
    Vector3 slideDir_{};
    float slamLandTimer_ = 0;       // window for boosted post-slam jump
    float fovKick_ = 0;
};
