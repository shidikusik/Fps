#pragma once

#include "raylib.h"
#include "player.h"
#include "weapons.h"

#include <vector>

// Maps window/screen coordinates into the letterboxed render target.
struct RenderMap {
    float offX = 0, offY = 0, scale = 1;
    Vector2 ToRender(Vector2 screen) const {
        return { (screen.x - offX) / scale, (screen.y - offY) / scale };
    }
};

// On-screen controls for touch devices: dynamic move stick on the left,
// look-drag on the right, action buttons. Enabled on Android, or anywhere
// with BLOODRUSH_TOUCH=1 for testing.
class TouchControls {
public:
    TouchControls();
    // Fills inputs from current touches; pausePressed reports the pause tap.
    void Gather(const RenderMap& map, PlayerInput& pin, CombatInput& cin,
                bool& pausePressed);
    void Draw() const;   // render-target coordinates

private:
    enum BtnId { BTN_FIRE, BTN_ALT, BTN_JUMP, BTN_DASH, BTN_CROUCH,
                 BTN_WPN, BTN_PAUSE, BTN_COUNT };
    struct Btn {
        Vector2 pos; float r;
        const char* labelEn; const char* labelRu;
        bool held = false, pressed = false;
    };
    enum Kind { KIND_STICK, KIND_LOOK, KIND_BTN };
    struct Tracked {
        int id; Kind kind; int btn;
        Vector2 start, last;
    };

    int FindButton(Vector2 p) const;

    Btn btns_[BTN_COUNT];
    std::vector<Tracked> touches_;
    Vector2 stickOrigin_{}, stickPos_{};
    bool stickActive_ = false;
};
