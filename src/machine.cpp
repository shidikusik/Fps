#include "machine.h"
#include "rlgl.h"

#include <cmath>

namespace machine {
namespace {
constexpr Color METAL  = { 58, 56, 66, 255 };
constexpr Color DARK   = { 28, 26, 32, 255 };
constexpr Color CORE   = { 230, 30, 40, 255 };
constexpr Color VISOR  = { 255, 230, 0, 255 };
constexpr Color WIRE   = { 120, 30, 40, 255 };
constexpr Color GUNMTL = { 82, 76, 84, 255 };

void Legs(float animT, float speed) {
    float step = sinf(animT * 2.2f) * (speed > 1 ? 0.28f : 0.02f);
    // pelvis
    DrawCube({ 0, 0.95f, 0 }, 0.52f, 0.22f, 0.34f, DARK);
    // thighs + shins, stepping opposite phases
    DrawCube({ 0.16f, 0.66f, step * 0.5f }, 0.2f, 0.42f, 0.24f, METAL);
    DrawCube({ -0.16f, 0.66f, -step * 0.5f }, 0.2f, 0.42f, 0.24f, METAL);
    DrawCube({ 0.16f, 0.24f, step }, 0.17f, 0.46f, 0.2f, DARK);
    DrawCube({ -0.16f, 0.24f, -step }, 0.17f, 0.46f, 0.2f, DARK);
    // feet
    DrawCube({ 0.16f, 0.05f, 0.06f + step }, 0.19f, 0.1f, 0.34f, METAL);
    DrawCube({ -0.16f, 0.05f, 0.06f - step }, 0.19f, 0.1f, 0.34f, METAL);
    DrawCubeWires({ 0, 0.95f, 0 }, 0.52f, 0.22f, 0.34f, WIRE);
}
} // namespace

void DrawLegsOnly(Vector3 pos, float yawDeg, float animT, float speed) {
    rlPushMatrix();
    rlTranslatef(pos.x, pos.y, pos.z);
    rlRotatef(yawDeg, 0, 1, 0);
    Legs(animT, speed);
    rlPopMatrix();
}

void Draw(Vector3 pos, float yawDeg, float animT, float scale) {
    float bob = sinf(animT * 1.3f) * 0.02f;
    float headSway = sinf(animT * 0.6f) * 6.0f;

    rlPushMatrix();
    rlTranslatef(pos.x, pos.y + bob, pos.z);
    rlRotatef(yawDeg, 0, 1, 0);
    rlScalef(scale, scale, scale);

    Legs(animT, 0);

    // torso: chest plate with glowing core
    DrawCube({ 0, 1.36f, 0 }, 0.62f, 0.6f, 0.4f, METAL);
    DrawCubeWires({ 0, 1.36f, 0 }, 0.62f, 0.6f, 0.4f, WIRE);
    DrawCube({ 0, 1.42f, 0.19f }, 0.44f, 0.34f, 0.06f, DARK);
    float pulse = 0.75f + 0.25f * sinf(animT * 2.6f);
    Color core = { (unsigned char)(CORE.r * pulse), (unsigned char)(CORE.g * pulse),
                   (unsigned char)(CORE.b * pulse), 255 };
    DrawCube({ 0, 1.42f, 0.22f }, 0.14f, 0.14f, 0.05f, core);

    // shoulders + arms (right hand rests on the revolver)
    DrawCube({ 0.42f, 1.58f, 0 }, 0.22f, 0.2f, 0.26f, DARK);
    DrawCube({ -0.42f, 1.58f, 0 }, 0.22f, 0.2f, 0.26f, DARK);
    DrawCube({ 0.44f, 1.22f, 0.04f }, 0.16f, 0.52f, 0.2f, METAL);
    DrawCube({ -0.44f, 1.22f, 0.0f }, 0.16f, 0.52f, 0.2f, METAL);
    // holstered revolver at the right hip
    DrawCube({ 0.36f, 0.92f, 0.16f }, 0.08f, 0.2f, 0.1f, GUNMTL);
    DrawCube({ 0.36f, 0.82f, 0.2f }, 0.06f, 0.1f, 0.16f, DARK);

    // head with visor
    rlPushMatrix();
    rlTranslatef(0, 1.85f, 0);
    rlRotatef(headSway, 0, 1, 0);
    DrawCube({ 0, 0, 0 }, 0.34f, 0.3f, 0.32f, METAL);
    DrawCubeWires({ 0, 0, 0 }, 0.34f, 0.3f, 0.32f, WIRE);
    DrawCube({ 0, 0.02f, 0.15f }, 0.26f, 0.08f, 0.05f, VISOR);
    DrawCube({ 0.1f, 0.2f, -0.08f }, 0.03f, 0.14f, 0.03f, DARK); // antenna
    rlPopMatrix();

    rlPopMatrix();
}

} // namespace machine
