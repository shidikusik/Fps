// BLOODRUSH — fast-paced arena FPS.
// Milestone 1: arena, movement (bhop / dash / slide / slam), pixel post-effect.

#include "raylib.h"
#include "raymath.h"

#include "arena.h"
#include "config.h"
#include "player.h"

#include <cmath>
#include <cstdio>

namespace {

constexpr Color HUD_RED    = { 230, 30, 40, 255 };
constexpr Color HUD_YELLOW = { 255, 230, 0, 255 };
constexpr Color HUD_WHITE  = { 235, 230, 230, 255 };
constexpr Color HUD_DIM    = { 120, 60, 64, 255 };

PlayerInput GatherInput() {
    PlayerInput in;
    Vector2 md = GetMouseDelta();
    in.mouseDx = md.x;
    in.mouseDy = md.y;
    in.fwd  = (IsKeyDown(KEY_W) ? 1.0f : 0.0f) - (IsKeyDown(KEY_S) ? 1.0f : 0.0f);
    in.side = (IsKeyDown(KEY_D) ? 1.0f : 0.0f) - (IsKeyDown(KEY_A) ? 1.0f : 0.0f);
    in.jumpPressed  = IsKeyPressed(KEY_SPACE);
    in.jumpHeld     = IsKeyDown(KEY_SPACE);
    in.dashPressed  = IsKeyPressed(KEY_LEFT_SHIFT) || IsKeyPressed(KEY_RIGHT_SHIFT);
    in.crouchPressed= IsKeyPressed(KEY_LEFT_CONTROL) || IsKeyPressed(KEY_RIGHT_CONTROL);
    in.crouchHeld   = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);
    return in;
}

void DrawHud(const Player& player) {
    const int W = cfg::RENDER_W, H = cfg::RENDER_H;

    // Crosshair
    DrawLine(W / 2 - 6, H / 2, W / 2 - 2, H / 2, HUD_WHITE);
    DrawLine(W / 2 + 2, H / 2, W / 2 + 6, H / 2, HUD_WHITE);
    DrawLine(W / 2, H / 2 - 6, W / 2, H / 2 - 2, HUD_WHITE);
    DrawLine(W / 2, H / 2 + 2, W / 2, H / 2 + 6, HUD_WHITE);

    // Speed readout (bottom left) — the movement dashboard
    float speed = player.HorizontalSpeed();
    char buf[64];
    snprintf(buf, sizeof(buf), "%5.1f", speed);
    Color speedCol = HUD_WHITE;
    if (speed > cfg::WALK_SPEED * 1.2f) speedCol = HUD_YELLOW;
    if (speed > cfg::DASH_SPEED * 0.8f) speedCol = HUD_RED;
    DrawText(buf, 16, H - 58, 40, speedCol);
    DrawText("UPS", 16 + MeasureText(buf, 40) + 8, H - 44, 20, HUD_DIM);

    // Speed bar
    float t = Clamp(speed / cfg::DASH_SPEED, 0.0f, 1.0f);
    DrawRectangle(16, H - 16, 180, 6, { 40, 20, 22, 255 });
    DrawRectangle(16, H - 16, (int)(180 * t), 6, speedCol);

    // Dash charges (bottom left, above speed)
    for (int i = 0; i < cfg::DASH_CHARGES; i++) {
        float fill = Clamp(player.dashCharges - (float)i, 0.0f, 1.0f);
        int x = 16 + i * 34;
        DrawRectangleLines(x, H - 84, 28, 12, HUD_DIM);
        if (fill > 0)
            DrawRectangle(x + 2, H - 82, (int)(24 * fill), 8,
                          fill >= 1.0f ? HUD_YELLOW : HUD_DIM);
    }

    // Movement state tag
    const char* state = nullptr;
    if (player.dashing) state = "DASH";
    else if (player.sliding) state = "SLIDE";
    else if (player.slamming) state = "SLAM";
    else if (!player.grounded) state = "AIR";
    if (state) DrawText(state, 16, H - 108, 20, HUD_RED);

    DrawFPS(W - 90, 8);
}

void DrawPauseOverlay() {
    const int W = cfg::RENDER_W, H = cfg::RENDER_H;
    DrawRectangle(0, 0, W, H, { 0, 0, 0, 160 });
    const char* title = "BLOODRUSH";
    DrawText(title, W / 2 - MeasureText(title, 60) / 2, H / 2 - 120, 60, HUD_RED);
    const char* sub = "PAUSED - click to resume";
    DrawText(sub, W / 2 - MeasureText(sub, 20) / 2, H / 2 - 40, 20, HUD_WHITE);

    const char* lines[] = {
        "WASD + mouse   move / look",
        "SPACE          jump (hold = bunny hop)",
        "SHIFT          dash (3 charges)",
        "CTRL           slide / ground slam in air",
        "ESC            pause",
    };
    for (int i = 0; i < 5; i++)
        DrawText(lines[i], W / 2 - 160, H / 2 + i * 24, 16, HUD_DIM);
}

} // namespace

int main() {
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);
    InitWindow(1280, 720, "BLOODRUSH");
    SetExitKey(KEY_NULL); // ESC pauses instead of quitting

    RenderTexture2D target = LoadRenderTexture(cfg::RENDER_W, cfg::RENDER_H);
    SetTextureFilter(target.texture, TEXTURE_FILTER_POINT); // crunchy pixels

    Arena arena;
    arena.Init();

    Player player;
    player.Init(arena.SpawnPoint());

    bool paused = false;
    DisableCursor();

    float shakeTime = 0;

    while (!WindowShouldClose()) {
        float dt = fminf(GetFrameTime(), cfg::MAX_DT);

        if (IsKeyPressed(KEY_ESCAPE)) {
            paused = !paused;
            if (paused) EnableCursor();
            else DisableCursor();
        }
        if (paused && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            paused = false;
            DisableCursor();
        }
        if (IsKeyPressed(KEY_F11)) ToggleFullscreen();

        if (!paused) {
            PlayerInput in = GatherInput();
            player.Update(in, arena, dt);
            shakeTime += dt * 40.0f;
        }

        // Screenshake: trauma^2 scaled noise on look angles
        float sh = player.trauma * player.trauma;
        float shakeX = sh * 2.2f * sinf(shakeTime * 1.13f) * cosf(shakeTime * 0.71f);
        float shakeY = sh * 2.2f * sinf(shakeTime * 0.97f + 1.7f);

        Camera3D cam = player.GetCamera(shakeX, shakeY);

        // --- render world + HUD into the low-res target ---
        BeginTextureMode(target);
        ClearBackground({ 8, 4, 6, 255 });
        BeginMode3D(cam);
        arena.Draw();
        EndMode3D();
        DrawHud(player);
        if (paused) DrawPauseOverlay();
        EndTextureMode();

        // --- upscale to window, integer-unfriendly sizes still stay sharp ---
        BeginDrawing();
        ClearBackground(BLACK);
        float scale = fminf((float)GetScreenWidth() / cfg::RENDER_W,
                            (float)GetScreenHeight() / cfg::RENDER_H);
        float outW = cfg::RENDER_W * scale, outH = cfg::RENDER_H * scale;
        Rectangle src = { 0, 0, (float)cfg::RENDER_W, -(float)cfg::RENDER_H };
        Rectangle dst = { (GetScreenWidth() - outW) * 0.5f,
                          (GetScreenHeight() - outH) * 0.5f, outW, outH };
        DrawTexturePro(target.texture, src, dst, { 0, 0 }, 0, WHITE);
        EndDrawing();
    }

    UnloadRenderTexture(target);
    CloseWindow();
    return 0;
}
