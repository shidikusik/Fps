#include "bindings.h"
#include "settings.h"
#include "raylib.h"

#include <cstdio>
#include <cstring>
#include <string>

namespace binds {
namespace {

Binding table[A_COUNT];

std::string Path() {
    std::string dir = settings::ConfigDir();
    if (dir.empty()) return "";
    return dir + "/bloodrush.binds";
}

} // namespace

Binding& Get(Action a) { return table[a]; }

void ResetDefaults() {
    table[A_FWD]      = { KEY_W, -1, -1 };
    table[A_BACK]     = { KEY_S, -1, -1 };
    table[A_LEFT]     = { KEY_A, -1, -1 };
    table[A_RIGHT]    = { KEY_D, -1, -1 };
    table[A_JUMP]     = { KEY_SPACE, -1, GAMEPAD_BUTTON_RIGHT_FACE_DOWN };   // CROSS
    table[A_DASH]     = { KEY_LEFT_SHIFT, -1, GAMEPAD_BUTTON_LEFT_TRIGGER_1 };  // L1
    table[A_CROUCH]   = { KEY_LEFT_CONTROL, -1, GAMEPAD_BUTTON_RIGHT_TRIGGER_1 }; // R1
    table[A_FIRE]     = { -1, MOUSE_BUTTON_LEFT, GAMEPAD_BUTTON_RIGHT_TRIGGER_2 }; // R2
    table[A_ALT]      = { -1, MOUSE_BUTTON_RIGHT, GAMEPAD_BUTTON_LEFT_TRIGGER_2 }; // L2
    table[A_WPN_NEXT] = { KEY_Q, -1, GAMEPAD_BUTTON_RIGHT_FACE_LEFT };       // SQUARE
    table[A_PAUSE]    = { KEY_ESCAPE, -1, GAMEPAD_BUTTON_MIDDLE_RIGHT };     // OPTIONS
}

void Save() {
    std::string p = Path();
    if (p.empty()) return;
    FILE* f = fopen(p.c_str(), "w");
    if (!f) return;
    for (int i = 0; i < A_COUNT; i++)
        fprintf(f, "%d %d %d\n", table[i].key, table[i].mouse, table[i].pad);
    fclose(f);
}

void Load() {
    ResetDefaults();
    std::string p = Path();
    if (p.empty()) return;
    FILE* f = fopen(p.c_str(), "r");
    if (!f) return;
    for (int i = 0; i < A_COUNT; i++) {
        int k, m, g;
        if (fscanf(f, "%d %d %d", &k, &m, &g) == 3)
            table[i] = { k, m, g };
    }
    fclose(f);
}

bool PadActive() { return IsGamepadAvailable(0); }

bool Down(Action a) {
    const Binding& b = table[a];
    if (b.key >= 0 && IsKeyDown(b.key)) return true;
    if (b.mouse >= 0 && IsMouseButtonDown(b.mouse)) return true;
    if (b.pad >= 0 && PadActive() && IsGamepadButtonDown(0, b.pad)) return true;
    return false;
}

bool Pressed(Action a) {
    const Binding& b = table[a];
    if (b.key >= 0 && IsKeyPressed(b.key)) return true;
    if (b.mouse >= 0 && IsMouseButtonPressed(b.mouse)) return true;
    if (b.pad >= 0 && PadActive() && IsGamepadButtonPressed(0, b.pad)) return true;
    return false;
}

const char* ActionName(Action a, bool ru) {
    static const char* en[A_COUNT] = {
        "FORWARD", "BACK", "LEFT", "RIGHT", "JUMP", "DASH", "SLIDE/SLAM",
        "FIRE", "ALT FIRE", "NEXT WEAPON", "PAUSE" };
    static const char* rus[A_COUNT] = {
        "ВПЕРЁД", "НАЗАД", "ВЛЕВО", "ВПРАВО", "ПРЫЖОК", "РЫВОК", "ПОДКАТ/УДАР",
        "ОГОНЬ", "АЛЬТ. ОГОНЬ", "СЛЕД. ОРУЖИЕ", "ПАУЗА" };
    return ru ? rus[a] : en[a];
}

const char* KeyLabel(const Binding& b) {
    static char buf[24];
    if (b.mouse == MOUSE_BUTTON_LEFT) return "LMB";
    if (b.mouse == MOUSE_BUTTON_RIGHT) return "RMB";
    if (b.mouse == MOUSE_BUTTON_MIDDLE) return "MMB";
    if (b.key < 0) return "—";
    int k = b.key;
    if (k >= KEY_A && k <= KEY_Z) { snprintf(buf, sizeof(buf), "%c", 'A' + (k - KEY_A)); return buf; }
    if (k >= KEY_ZERO && k <= KEY_NINE) { snprintf(buf, sizeof(buf), "%c", '0' + (k - KEY_ZERO)); return buf; }
    switch (k) {
        case KEY_SPACE: return "SPACE";
        case KEY_LEFT_SHIFT: return "LSHIFT";
        case KEY_RIGHT_SHIFT: return "RSHIFT";
        case KEY_LEFT_CONTROL: return "LCTRL";
        case KEY_RIGHT_CONTROL: return "RCTRL";
        case KEY_LEFT_ALT: return "LALT";
        case KEY_TAB: return "TAB";
        case KEY_ENTER: return "ENTER";
        case KEY_ESCAPE: return "ESC";
        case KEY_BACKSPACE: return "BKSP";
        case KEY_UP: return "UP";
        case KEY_DOWN: return "DOWN";
        case KEY_LEFT: return "LEFT";
        case KEY_RIGHT: return "RIGHT";
        case KEY_F: return "F";
        case KEY_CAPS_LOCK: return "CAPS";
    }
    snprintf(buf, sizeof(buf), "KEY%d", k);
    return buf;
}

const char* PadLabel(const Binding& b) {
    switch (b.pad) {
        case -1: return "—";
        case GAMEPAD_BUTTON_RIGHT_FACE_DOWN: return "CROSS";
        case GAMEPAD_BUTTON_RIGHT_FACE_RIGHT: return "CIRCLE";
        case GAMEPAD_BUTTON_RIGHT_FACE_LEFT: return "SQUARE";
        case GAMEPAD_BUTTON_RIGHT_FACE_UP: return "TRIANGLE";
        case GAMEPAD_BUTTON_LEFT_TRIGGER_1: return "L1";
        case GAMEPAD_BUTTON_LEFT_TRIGGER_2: return "L2";
        case GAMEPAD_BUTTON_RIGHT_TRIGGER_1: return "R1";
        case GAMEPAD_BUTTON_RIGHT_TRIGGER_2: return "R2";
        case GAMEPAD_BUTTON_MIDDLE_LEFT: return "SHARE";
        case GAMEPAD_BUTTON_MIDDLE_RIGHT: return "OPTIONS";
        case GAMEPAD_BUTTON_MIDDLE: return "PS";
        case GAMEPAD_BUTTON_LEFT_THUMB: return "L3";
        case GAMEPAD_BUTTON_RIGHT_THUMB: return "R3";
        case GAMEPAD_BUTTON_LEFT_FACE_UP: return "DPAD UP";
        case GAMEPAD_BUTTON_LEFT_FACE_DOWN: return "DPAD DN";
        case GAMEPAD_BUTTON_LEFT_FACE_LEFT: return "DPAD L";
        case GAMEPAD_BUTTON_LEFT_FACE_RIGHT: return "DPAD R";
    }
    static char buf[16];
    snprintf(buf, sizeof(buf), "PAD%d", b.pad);
    return buf;
}

} // namespace binds
