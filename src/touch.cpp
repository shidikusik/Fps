#include "touch.h"
#include "config.h"
#include "raymath.h"

#include <algorithm>
#include <cmath>

namespace {
constexpr float STICK_ZONE_X = 480;   // left of this = movement stick
constexpr float STICK_RANGE = 70;     // px from origin for full deflection
constexpr float LOOK_SENS = 2.4f;     // touch px -> "mouse" px
constexpr Color UI_DIM = { 235, 230, 230, 60 };
constexpr Color UI_LINE = { 235, 230, 230, 110 };
constexpr Color UI_HOT = { 255, 230, 0, 160 };
} // namespace

TouchControls::TouchControls() {
    const float W = (float)cfg::RENDER_W, H = (float)cfg::RENDER_H;
    btns_[BTN_FIRE]   = { { W - 110, H - 150 }, 72, "FIRE" };
    btns_[BTN_ALT]    = { { W - 260, H - 80 },  46, "ALT" };
    btns_[BTN_JUMP]   = { { W - 100, H - 325 }, 56, "JUMP" };
    btns_[BTN_DASH]   = { { W - 240, H - 240 }, 46, "DASH" };
    btns_[BTN_CROUCH] = { { W - 375, H - 135 }, 46, "CRCH" };
    btns_[BTN_WPN]    = { { W - 70,  H - 475 }, 42, "WPN" };
    btns_[BTN_PAUSE]  = { { 45, 45 },           32, "II" };
}

int TouchControls::FindButton(Vector2 p) const {
    for (int i = 0; i < BTN_COUNT; i++)
        if (Vector2Distance(p, btns_[i].pos) <= btns_[i].r * 1.25f) return i;
    return -1;
}

void TouchControls::Gather(const RenderMap& map, PlayerInput& pin,
                           CombatInput& cin, bool& pausePressed) {
    for (auto& b : btns_) { b.held = false; b.pressed = false; }
    float lookDx = 0, lookDy = 0;

    // current touches (desktop fallback: mouse acts as touch id 0)
    struct Cur { int id; Vector2 pos; };
    std::vector<Cur> cur;
    int n = GetTouchPointCount();
    for (int i = 0; i < n; i++)
        cur.push_back({ GetTouchPointId(i), map.ToRender(GetTouchPosition(i)) });
    if (n == 0 && IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        cur.push_back({ 0, map.ToRender(GetMousePosition()) });

    // match/track
    for (const Cur& c : cur) {
        auto it = std::find_if(touches_.begin(), touches_.end(),
                               [&](const Tracked& t) { return t.id == c.id; });
        if (it == touches_.end()) {
            Tracked t;
            t.id = c.id;
            t.start = t.last = c.pos;
            int b = FindButton(c.pos);
            if (b >= 0) {
                t.kind = KIND_BTN;
                t.btn = b;
                btns_[b].pressed = true;
            } else if (c.pos.x < STICK_ZONE_X) {
                t.kind = KIND_STICK;
                t.btn = -1;
                stickOrigin_ = c.pos;
            } else {
                t.kind = KIND_LOOK;
                t.btn = -1;
            }
            touches_.push_back(t);
        } else {
            if (it->kind == KIND_LOOK) {
                lookDx += c.pos.x - it->last.x;
                lookDy += c.pos.y - it->last.y;
            }
            it->last = c.pos;
        }
    }
    // drop ended touches
    touches_.erase(std::remove_if(touches_.begin(), touches_.end(),
        [&](const Tracked& t) {
            return std::none_of(cur.begin(), cur.end(),
                                [&](const Cur& c) { return c.id == t.id; });
        }), touches_.end());

    // held states
    stickActive_ = false;
    for (const Tracked& t : touches_) {
        if (t.kind == KIND_BTN) btns_[t.btn].held = true;
        if (t.kind == KIND_STICK) {
            stickActive_ = true;
            stickPos_ = t.last;
        }
    }

    // --- fill inputs ---
    pin = PlayerInput{};
    if (stickActive_) {
        Vector2 d = Vector2Subtract(stickPos_, stickOrigin_);
        float len = Vector2Length(d);
        if (len > STICK_RANGE) d = Vector2Scale(d, STICK_RANGE / len);
        pin.side = Clamp(d.x / STICK_RANGE, -1.0f, 1.0f);
        pin.fwd = Clamp(-d.y / STICK_RANGE, -1.0f, 1.0f);
    }
    pin.mouseDx = lookDx * LOOK_SENS;
    pin.mouseDy = lookDy * LOOK_SENS;
    pin.jumpPressed = btns_[BTN_JUMP].pressed;
    pin.jumpHeld = btns_[BTN_JUMP].held;
    pin.dashPressed = btns_[BTN_DASH].pressed;
    pin.crouchPressed = btns_[BTN_CROUCH].pressed;
    pin.crouchHeld = btns_[BTN_CROUCH].held;

    cin = CombatInput{};
    cin.fireHeld = btns_[BTN_FIRE].held;
    cin.altHeld = btns_[BTN_ALT].held;
    cin.cycle = btns_[BTN_WPN].pressed ? 1 : 0;
    cin.lookDx = pin.mouseDx;
    cin.lookDy = pin.mouseDy;

    pausePressed = btns_[BTN_PAUSE].pressed;
}

void TouchControls::Draw() const {
    // movement stick
    if (stickActive_) {
        DrawCircleLines((int)stickOrigin_.x, (int)stickOrigin_.y, STICK_RANGE, UI_LINE);
        Vector2 d = Vector2Subtract(stickPos_, stickOrigin_);
        float len = Vector2Length(d);
        if (len > STICK_RANGE) d = Vector2Scale(d, STICK_RANGE / len);
        Vector2 nub = Vector2Add(stickOrigin_, d);
        DrawCircle((int)nub.x, (int)nub.y, 26, UI_DIM);
        DrawCircleLines((int)nub.x, (int)nub.y, 26, UI_HOT);
    } else {
        DrawCircleLines(180, cfg::RENDER_H - 170, 40, UI_DIM);
        DrawText("MOVE", 180 - MeasureText("MOVE", 14) / 2,
                 cfg::RENDER_H - 170 - 7, 14, UI_DIM);
    }

    for (const Btn& b : btns_) {
        Color line = b.held ? UI_HOT : UI_LINE;
        DrawCircle((int)b.pos.x, (int)b.pos.y, b.r, Fade(BLACK, 0.25f));
        DrawCircleLines((int)b.pos.x, (int)b.pos.y, b.r, line);
        int fs = b.r >= 55 ? 18 : 14;
        DrawText(b.label, (int)b.pos.x - MeasureText(b.label, fs) / 2,
                 (int)b.pos.y - fs / 2, fs, line);
    }
}
