#include "machine.h"
#include "rlgl.h"

#include <cmath>

namespace machine {
namespace {
constexpr Color METAL   = { 58, 56, 66, 255 };
constexpr Color METAL_L = { 74, 72, 84, 255 };
constexpr Color DARK    = { 28, 26, 32, 255 };
constexpr Color CORE    = { 230, 30, 40, 255 };
constexpr Color VISOR   = { 255, 230, 0, 255 };
constexpr Color WIRE    = { 120, 30, 40, 255 };
constexpr Color GUNMTL  = { 82, 76, 84, 255 };
constexpr Color SEAM    = { 200, 40, 50, 255 };

void Legs(float animT, float speed) {
    float step = sinf(animT * 2.2f) * (speed > 1 ? 0.28f : 0.02f);
    // pelvis with hip joints
    DrawCube({ 0, 0.95f, 0 }, 0.52f, 0.22f, 0.34f, DARK);
    DrawCube({ 0.17f, 0.9f, 0 }, 0.2f, 0.14f, 0.2f, METAL_L);
    DrawCube({ -0.17f, 0.9f, 0 }, 0.2f, 0.14f, 0.2f, METAL_L);
    // glowing seam across the pelvis
    DrawCube({ 0, 0.98f, 0.18f }, 0.4f, 0.03f, 0.02f, SEAM);
    // thighs, knee joints, shins — stepping opposite phases
    DrawCube({ 0.16f, 0.66f, step * 0.5f }, 0.2f, 0.42f, 0.24f, METAL);
    DrawCube({ -0.16f, 0.66f, -step * 0.5f }, 0.2f, 0.42f, 0.24f, METAL);
    DrawCube({ 0.16f, 0.46f, step * 0.75f }, 0.16f, 0.12f, 0.18f, METAL_L); // knees
    DrawCube({ -0.16f, 0.46f, -step * 0.75f }, 0.16f, 0.12f, 0.18f, METAL_L);
    DrawCube({ 0.16f, 0.24f, step }, 0.17f, 0.46f, 0.2f, DARK);
    DrawCube({ -0.16f, 0.24f, -step }, 0.17f, 0.46f, 0.2f, DARK);
    // feet with toe plates
    DrawCube({ 0.16f, 0.05f, 0.06f + step }, 0.19f, 0.1f, 0.34f, METAL);
    DrawCube({ -0.16f, 0.05f, 0.06f - step }, 0.19f, 0.1f, 0.34f, METAL);
    DrawCube({ 0.16f, 0.05f, 0.24f + step }, 0.15f, 0.06f, 0.08f, VISOR);
    DrawCube({ -0.16f, 0.05f, 0.24f - step }, 0.15f, 0.06f, 0.08f, VISOR);
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
    float breathe = sinf(animT * 1.3f) * 0.008f;
    float headSway = sinf(animT * 0.6f) * 6.0f;

    rlPushMatrix();
    rlTranslatef(pos.x, pos.y + bob, pos.z);
    rlRotatef(yawDeg, 0, 1, 0);
    rlScalef(scale, scale, scale);

    Legs(animT, 0);

    // torso: layered chest with vents and glowing core
    DrawCube({ 0, 1.36f + breathe, 0 }, 0.62f, 0.6f, 0.4f, METAL);
    DrawCubeWires({ 0, 1.36f + breathe, 0 }, 0.62f, 0.6f, 0.4f, WIRE);
    DrawCube({ 0, 1.42f + breathe, 0.19f }, 0.44f, 0.34f, 0.06f, DARK);
    float pulse = 0.75f + 0.25f * sinf(animT * 2.6f);
    Color core = { (unsigned char)(CORE.r * pulse), (unsigned char)(CORE.g * pulse),
                   (unsigned char)(CORE.b * pulse), 255 };
    DrawCube({ 0, 1.42f + breathe, 0.22f }, 0.14f, 0.14f, 0.05f, core);
    // chest vents (three slats each side of the core)
    for (int i = 0; i < 3; i++) {
        DrawCube({ 0.19f, 1.32f + breathe + i * 0.08f, 0.21f }, 0.1f, 0.03f, 0.03f, DARK);
        DrawCube({ -0.19f, 1.32f + breathe + i * 0.08f, 0.21f }, 0.1f, 0.03f, 0.03f, DARK);
    }
    // waist seam + back thruster pack
    DrawCube({ 0, 1.08f, 0 }, 0.4f, 0.1f, 0.3f, METAL_L);
    DrawCube({ 0, 1.42f, -0.25f }, 0.4f, 0.44f, 0.14f, DARK);
    DrawCube({ 0.1f, 1.3f, -0.33f }, 0.1f, 0.16f, 0.05f, core);   // thruster glow
    DrawCube({ -0.1f, 1.3f, -0.33f }, 0.1f, 0.16f, 0.05f, core);

    // shoulders with lamps, segmented arms
    DrawCube({ 0.44f, 1.6f, 0 }, 0.26f, 0.22f, 0.28f, METAL_L);
    DrawCube({ -0.44f, 1.6f, 0 }, 0.26f, 0.22f, 0.28f, METAL_L);
    DrawCube({ 0.44f, 1.72f, 0.08f }, 0.08f, 0.04f, 0.06f, VISOR); // lamps
    DrawCube({ -0.44f, 1.72f, 0.08f }, 0.08f, 0.04f, 0.06f, VISOR);
    // left arm hangs, right arm bent holding the revolver
    DrawCube({ -0.46f, 1.28f, 0.0f }, 0.16f, 0.42f, 0.2f, METAL);
    DrawCube({ -0.46f, 1.04f, 0.02f }, 0.14f, 0.1f, 0.16f, METAL_L); // elbow
    DrawCube({ -0.46f, 0.9f, 0.05f }, 0.13f, 0.26f, 0.16f, DARK);
    DrawCube({ 0.46f, 1.34f, 0.05f }, 0.16f, 0.32f, 0.2f, METAL);
    DrawCube({ 0.46f, 1.16f, 0.14f }, 0.14f, 0.1f, 0.16f, METAL_L);  // elbow
    DrawCube({ 0.46f, 1.12f, 0.3f }, 0.13f, 0.14f, 0.3f, DARK);      // forearm fwd
    // revolver in the right hand
    DrawCube({ 0.46f, 1.16f, 0.52f }, 0.05f, 0.07f, 0.3f, GUNMTL);   // barrel
    DrawCube({ 0.46f, 1.1f, 0.42f }, 0.07f, 0.1f, 0.12f, DARK);      // frame
    DrawCube({ 0.46f, 1.22f, 0.64f }, 0.02f, 0.03f, 0.03f, VISOR);   // sight

    // head with visor, jaw, antenna
    rlPushMatrix();
    rlTranslatef(0, 1.88f, 0);
    rlRotatef(headSway, 0, 1, 0);
    DrawCube({ 0, 0, 0 }, 0.34f, 0.3f, 0.32f, METAL);
    DrawCubeWires({ 0, 0, 0 }, 0.34f, 0.3f, 0.32f, WIRE);
    float vPulse = 0.8f + 0.2f * sinf(animT * 4.0f);
    Color visor = { (unsigned char)(VISOR.r * vPulse), (unsigned char)(VISOR.g * vPulse),
                    0, 255 };
    DrawCube({ 0, 0.02f, 0.15f }, 0.26f, 0.08f, 0.05f, visor);
    DrawCube({ 0, -0.11f, 0.13f }, 0.2f, 0.06f, 0.06f, DARK);        // jaw
    DrawCube({ 0.12f, 0.09f, 0.16f }, 0.04f, 0.04f, 0.02f, SEAM);    // cheek light
    DrawCube({ 0.1f, 0.22f, -0.08f }, 0.03f, 0.16f, 0.03f, DARK);    // antenna
    DrawCube({ 0.1f, 0.31f, -0.08f }, 0.05f, 0.04f, 0.05f, core);    // antenna tip
    rlPopMatrix();

    rlPopMatrix();
}

} // namespace machine
