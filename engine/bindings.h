#pragma once

// Rebindable input: every action can have a keyboard key, a mouse button
// and a gamepad button. Checked together, so pads work everywhere the
// keyboard does. Persists next to the settings file.
namespace binds {

enum Action {
    A_FWD, A_BACK, A_LEFT, A_RIGHT,
    A_JUMP, A_DASH, A_CROUCH,
    A_FIRE, A_ALT, A_WPN_NEXT, A_PAUSE,
    A_COUNT
};

struct Binding {
    int key = -1;    // raylib KeyboardKey
    int mouse = -1;  // raylib MouseButton
    int pad = -1;    // raylib GamepadButton
};

Binding& Get(Action a);
void ResetDefaults();
void Load();
void Save();

bool Down(Action a);
bool Pressed(Action a);

// UI helpers
const char* ActionName(Action a, bool ru);
const char* KeyLabel(const Binding& b);   // "W" / "LMB" / "—"
const char* PadLabel(const Binding& b);   // "CROSS" / "R2" / "—"

// Gamepad discovery. PadIndex() scans slots 0..3 and returns the first
// connected pad (or -1). PadActive() is a convenience for "any pad".
int PadIndex();
bool PadActive();
const char* PadName();   // "" when none

} // namespace binds
