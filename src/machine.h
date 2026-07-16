#pragma once

#include "raylib.h"

// THE MACHINE — the player character model, built from primitives.
// Shown in the menu and cutscenes; in first person only the lower body
// is drawn (when looking down).
namespace machine {

// pos = feet center. animT drives idle motion.
void Draw(Vector3 pos, float yawDeg, float animT, float scale = 1.0f);
void DrawLegsOnly(Vector3 pos, float yawDeg, float animT, float speed);

} // namespace machine
