#include "player.h"
#include "arena.h"
#include "config.h"
#include "sounds.h"
#include "raymath.h"

#include <algorithm>
#include <cmath>

using namespace cfg;

void Player::Init(Vector3 spawn) {
    spawn_ = spawn;
    pos = spawn;
    vel = { 0, 0, 0 };
    yaw = 0.0f; // spawn at +Z looking toward the central tower (-Z)
    pitch = 0;
    grounded = false;
    sliding = dashing = slamming = false;
    dashCharges = (float)DASH_CHARGES;
    height_ = PLAYER_HEIGHT;
    trauma = 0;
    jumpBuffer_ = coyote_ = dashTimeLeft_ = slamLandTimer_ = 0;
    hp = maxHp;
    hurtFlash = healFlash = 0;
    hurtCd_ = 0;
    timeSinceDash = 99;
    slamLandedThisFrame = false;
}

Vector3 Player::EyePos() const {
    return { pos.x, pos.y + height_ - EYE_OFFSET, pos.z };
}

bool Player::TakeDamage(float dmg) {
    if (hurtCd_ > 0 || hp <= 0) return false;
    hp -= dmg;
    hurtCd_ = 0.45f;
    hurtFlash = 1.0f;
    AddTrauma(0.3f);
    sfx::Play(sfx::HURT);
    return true;
}

void Player::Heal(float amount) {
    if (hp <= 0 || hp >= maxHp) return;
    hp = std::min(maxHp, hp + amount);
    healFlash = 1.0f;
    sfx::Play(sfx::HEAL, 0.5f);
}

Vector3 Player::Forward() const {
    float cy = cosf(DEG2RAD * yaw), sy = sinf(DEG2RAD * yaw);
    float cp = cosf(DEG2RAD * pitch), sp = sinf(DEG2RAD * pitch);
    return { sy * cp, sp, -cy * cp };
}

Vector3 Player::FlatForward() const {
    float cy = cosf(DEG2RAD * yaw), sy = sinf(DEG2RAD * yaw);
    return { sy, 0, -cy };
}

float Player::HorizontalSpeed() const {
    return sqrtf(vel.x * vel.x + vel.z * vel.z);
}

void Player::AddTrauma(float t) {
    trauma = std::min(1.0f, trauma + t);
}

// Quake-style acceleration: only adds velocity up to wishspeed *projected on
// wishdir*, which is what makes strafe-jumping gain speed.
void Player::Accelerate(Vector3 wishdir, float wishspeed, float accel, float dt) {
    float current = vel.x * wishdir.x + vel.z * wishdir.z;
    float add = wishspeed - current;
    if (add <= 0) return;
    float speed = std::min(accel * wishspeed * dt, add);
    vel.x += wishdir.x * speed;
    vel.z += wishdir.z * speed;
}

void Player::ApplyFriction(float friction, float dt) {
    float speed = HorizontalSpeed();
    if (speed < 0.1f) { vel.x = vel.z = 0; return; }
    float drop = speed * friction * dt;
    float scale = std::max(speed - drop, 0.0f) / speed;
    vel.x *= scale;
    vel.z *= scale;
}

void Player::StartDash(Vector3 dir) {
    dashing = true;
    slamming = false;
    dashTimeLeft_ = DASH_TIME;
    dashDir_ = dir;
    dashCharges -= 1.0f;
    timeSinceDash = 0;
    AddTrauma(0.15f);
    sfx::Play(sfx::DASH, 0.7f);
}

void Player::StartSlide() {
    sliding = true;
    Vector3 dir = FlatForward();
    if (HorizontalSpeed() > 1.0f) {
        float inv = 1.0f / HorizontalSpeed();
        dir = { vel.x * inv, 0, vel.z * inv };
    }
    slideDir_ = dir;
    float speed = std::max(HorizontalSpeed() + SLIDE_BOOST, SLIDE_MIN_SPEED);
    vel.x = dir.x * speed;
    vel.z = dir.z * speed;
}

void Player::EndSlide() {
    sliding = false;
}

void Player::Update(const PlayerInput& in, const Arena& arena, float dt) {
    // --- look ---
    yaw += in.mouseDx * MOUSE_SENS;
    pitch = Clamp(pitch - in.mouseDy * MOUSE_SENS, -89.0f, 89.0f);

    // --- timers ---
    trauma = std::max(0.0f, trauma - dt * 2.2f);
    hurtFlash = std::max(0.0f, hurtFlash - dt * 2.5f);
    healFlash = std::max(0.0f, healFlash - dt * 3.0f);
    hurtCd_ = std::max(0.0f, hurtCd_ - dt);
    timeSinceDash += dt;
    slamLandedThisFrame = false;
    slamLandTimer_ = std::max(0.0f, slamLandTimer_ - dt);
    coyote_ = grounded ? COYOTE_TIME : std::max(0.0f, coyote_ - dt);
    if (dashCharges < (float)DASH_CHARGES)
        dashCharges = std::min((float)DASH_CHARGES, dashCharges + dt / DASH_RECHARGE);

    // --- wish direction from WASD, relative to yaw ---
    Vector3 fwd = FlatForward();
    Vector3 right = { -fwd.z, 0, fwd.x };
    Vector3 wishdir = {
        fwd.x * in.fwd + right.x * in.side, 0,
        fwd.z * in.fwd + right.z * in.side };
    float wishlen = Vector3Length(wishdir);
    if (wishlen > 0.001f) wishdir = Vector3Scale(wishdir, 1.0f / wishlen);

    // --- dash ---
    if (in.dashPressed && dashCharges >= 1.0f && !dashing) {
        StartDash(wishlen > 0.001f ? wishdir : fwd);
        if (sliding) EndSlide();
    }

    // --- crouch: slide on ground, slam in air ---
    if (in.crouchPressed) {
        if (grounded && !dashing) {
            StartSlide();
        } else if (!grounded && !slamming) {
            slamming = true;
            dashing = false;
            vel = { 0, -SLAM_SPEED, 0 };
            AddTrauma(0.1f);
        }
    }
    if (sliding && (!in.crouchHeld || !grounded)) EndSlide();

    // --- jump (buffered; holding space auto-bhops) ---
    if (in.jumpPressed || in.jumpHeld) jumpBuffer_ = JUMP_BUFFER;
    else jumpBuffer_ = std::max(0.0f, jumpBuffer_ - dt);

    bool jumped = false;
    if (jumpBuffer_ > 0 && (grounded || coyote_ > 0) && !slamming) {
        float mult = slamLandTimer_ > 0 ? SLAM_JUMP_MULT : 1.0f;
        vel.y = JUMP_VELOCITY * mult;
        if (dashing) { // dash-jump: keep the full dash velocity as momentum
            vel.x = dashDir_.x * DASH_SPEED;
            vel.z = dashDir_.z * DASH_SPEED;
            dashing = false;
        }
        if (sliding) EndSlide(); // slide-jump keeps speed
        grounded = false;
        coyote_ = 0;
        jumpBuffer_ = 0;
        jumped = true;
    }

    // --- physics ---
    if (dashing) {
        vel.x = dashDir_.x * DASH_SPEED;
        vel.z = dashDir_.z * DASH_SPEED;
        vel.y = 0; // dash freezes gravity
        dashTimeLeft_ -= dt;
        if (dashTimeLeft_ <= 0) {
            dashing = false;
            float sp = HorizontalSpeed();
            if (sp > DASH_END_KEEP) { // ended without a dash-jump: bleed excess
                float s = DASH_END_KEEP / sp;
                vel.x *= s;
                vel.z *= s;
            }
        }
    } else if (slamming) {
        vel = { 0, -SLAM_SPEED, 0 };
    } else {
        vel.y -= GRAVITY * dt;
        if (grounded && !jumped) {
            if (sliding) {
                ApplyFriction(SLIDE_FRICTION, dt);
                // steer without gaining speed: wishspeed = current speed
                if (wishlen > 0.001f)
                    Accelerate(wishdir, HorizontalSpeed(), SLIDE_STEER, dt);
            } else {
                ApplyFriction(GROUND_FRICTION, dt);
                if (wishlen > 0.001f)
                    Accelerate(wishdir, WALK_SPEED, GROUND_ACCEL, dt);
            }
        } else if (!grounded) {
            // air control: capped wishspeed is what enables bhop speed gain
            if (wishlen > 0.001f)
                Accelerate(wishdir, AIR_WISHSPEED_CAP, AIR_ACCEL, dt);
        }
    }

    // --- crouch height (smooth) ---
    float targetH = sliding ? PLAYER_CROUCH_H : PLAYER_HEIGHT;
    height_ += (targetH - height_) * std::min(1.0f, dt * 14.0f);

    // --- integrate + collide ---
    bool wasGrounded = grounded;
    MoveAndCollide(arena, dt);

    if (grounded && !wasGrounded) { // landed this frame
        if (slamming) {
            slamming = false;
            slamLandTimer_ = SLAM_JUMP_WINDOW;
            slamLandedThisFrame = true;
            AddTrauma(0.45f);
            sfx::Play(sfx::SLAM);
        }
    }

    // --- fell off the world ---
    if (pos.y < KILL_PLANE) {
        pos = spawn_;
        vel = { 0, 0, 0 };
        slamming = dashing = sliding = false;
        AddTrauma(0.3f);
    }
}

void Player::MoveAndCollide(const Arena& arena, float dt) {
    const float hw = PLAYER_HALF_W;
    auto overlaps = [&](const BoundingBox& b) {
        return pos.x + hw > b.min.x && pos.x - hw < b.max.x &&
               pos.y + height_ > b.min.y && pos.y < b.max.y &&
               pos.z + hw > b.min.z && pos.z - hw < b.max.z;
    };

    // X axis
    pos.x += vel.x * dt;
    for (const Block& blk : arena.Blocks()) {
        if (!overlaps(blk.box)) continue;
        float bcx = (blk.box.min.x + blk.box.max.x) * 0.5f;
        pos.x = (pos.x < bcx) ? blk.box.min.x - hw : blk.box.max.x + hw;
        vel.x = 0;
    }
    // Z axis
    pos.z += vel.z * dt;
    for (const Block& blk : arena.Blocks()) {
        if (!overlaps(blk.box)) continue;
        float bcz = (blk.box.min.z + blk.box.max.z) * 0.5f;
        pos.z = (pos.z < bcz) ? blk.box.min.z - hw : blk.box.max.z + hw;
        vel.z = 0;
    }
    // Y axis
    pos.y += vel.y * dt;
    grounded = false;
    for (const Block& blk : arena.Blocks()) {
        if (!overlaps(blk.box)) continue;
        float bcy = (blk.box.min.y + blk.box.max.y) * 0.5f;
        if (pos.y + height_ * 0.5f < bcy) { // hit ceiling
            pos.y = blk.box.min.y - height_;
            vel.y = std::min(vel.y, 0.0f);
        } else {                            // landed on top
            pos.y = blk.box.max.y;
            if (vel.y <= 0) {
                vel.y = 0;
                grounded = true;
            }
        }
    }
}

Camera3D Player::GetCamera(float shakeOffX, float shakeOffY) const {
    Camera3D cam{};
    float eyeY = pos.y + height_ - EYE_OFFSET;
    cam.position = { pos.x, eyeY, pos.z };

    float sy = yaw + shakeOffX;
    float sp = Clamp(pitch + shakeOffY, -89.0f, 89.0f);
    float cy = cosf(DEG2RAD * sy), syn = sinf(DEG2RAD * sy);
    float cp = cosf(DEG2RAD * sp), spn = sinf(DEG2RAD * sp);
    Vector3 fwd = { syn * cp, spn, -cy * cp };

    cam.target = Vector3Add(cam.position, fwd);
    cam.up = { 0, 1, 0 };

    // FOV widens with speed for a sense of velocity
    float speedT = Clamp((HorizontalSpeed() - WALK_SPEED) / 25.0f, 0.0f, 1.0f);
    cam.fovy = BASE_FOV + MAX_FOV_KICK * speedT;
    cam.projection = CAMERA_PERSPECTIVE;
    return cam;
}
