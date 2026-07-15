// BLOODRUSH — fast-paced arena FPS.
// Movement (bhop/dash/slide/slam) + revolver/shotgun + 3 enemy types,
// wave system, style meter, blood healing, procedural sound, game menu.

#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

#include "arena.h"
#include "config.h"
#include "enemies.h"
#include "particles.h"
#include "player.h"
#include "shading.h"
#include "sounds.h"
#include "style_meter.h"
#include "weapons.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>

namespace {

constexpr Color HUD_RED    = { 230, 30, 40, 255 };
constexpr Color HUD_YELLOW = { 255, 230, 0, 255 };
constexpr Color HUD_WHITE  = { 235, 230, 230, 255 };
constexpr Color HUD_DIM    = { 120, 60, 64, 255 };

enum class GameState { Menu, Playing, Paused, Dead };

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

void DrawCenteredText(const char* text, int y, int size, Color col) {
    DrawText(text, cfg::RENDER_W / 2 - MeasureText(text, size) / 2, y, size, col);
}

void DrawGameHud(const Player& player, const EnemyManager& enemies,
                 const StyleMeter& style) {
    const int W = cfg::RENDER_W, H = cfg::RENDER_H;

    // Crosshair
    DrawLine(W / 2 - 6, H / 2, W / 2 - 2, H / 2, HUD_WHITE);
    DrawLine(W / 2 + 2, H / 2, W / 2 + 6, H / 2, HUD_WHITE);
    DrawLine(W / 2, H / 2 - 6, W / 2, H / 2 - 2, HUD_WHITE);
    DrawLine(W / 2, H / 2 + 2, W / 2, H / 2 + 6, HUD_WHITE);

    // Speed (bottom left)
    float speed = player.HorizontalSpeed();
    char buf[64];
    snprintf(buf, sizeof(buf), "%5.1f", speed);
    Color speedCol = HUD_WHITE;
    if (speed > cfg::WALK_SPEED * 1.2f) speedCol = HUD_YELLOW;
    if (speed > cfg::DASH_SPEED * 0.8f) speedCol = HUD_RED;
    DrawText(buf, 16, H - 58, 40, speedCol);
    DrawText("UPS", 16 + MeasureText(buf, 40) + 8, H - 44, 20, HUD_DIM);
    float t = Clamp(speed / cfg::DASH_SPEED, 0.0f, 1.0f);
    DrawRectangle(16, H - 16, 180, 6, { 40, 20, 22, 255 });
    DrawRectangle(16, H - 16, (int)(180 * t), 6, speedCol);

    // Dash charges
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

    // Health (bottom center) — blood is fuel, it does not regen
    int hp = (int)ceilf(player.hp);
    Color hpCol = hp > 30 ? HUD_RED : Color{ 255, 60, 30, 255 };
    float pulse = hp <= 30 ? (0.5f + 0.5f * sinf((float)GetTime() * 9)) : 1.0f;
    snprintf(buf, sizeof(buf), "%d", hp < 0 ? 0 : hp);
    int bx = W / 2 - 110;
    DrawText(buf, bx - MeasureText(buf, 34) - 10, H - 46, 34,
             Fade(hpCol, pulse));
    DrawRectangle(bx, H - 34, 220, 12, { 40, 20, 22, 255 });
    DrawRectangle(bx, H - 34, (int)(220 * Clamp(player.hp / player.maxHp, 0.0f, 1.0f)),
                  12, Fade(hpCol, pulse));
    DrawRectangleLines(bx - 1, H - 35, 222, 14, HUD_DIM);
    if (player.healFlash > 0)
        DrawText("+BLOOD", bx + 228, H - 40, 16, Fade(HUD_YELLOW, player.healFlash));

    // Wave + score (top)
    if (enemies.waveActive) {
        snprintf(buf, sizeof(buf), "WAVE %d — %d LEFT", enemies.wave, enemies.AliveCount());
        DrawCenteredText(buf, 10, 20, HUD_WHITE);
    } else {
        snprintf(buf, sizeof(buf), "WAVE %d IN %d", enemies.wave + 1,
                 (int)ceilf(enemies.intermission));
        DrawCenteredText(buf, 10, 26, HUD_YELLOW);
        if (enemies.wave > 0)
            DrawCenteredText("WAVE CLEARED", 40, 16, HUD_DIM);
    }
    snprintf(buf, sizeof(buf), "%ld", style.score);
    DrawText(buf, W - MeasureText(buf, 26) - 16, 10, 26, HUD_YELLOW);

    DrawFPS(8, 8);
}

void DrawMenu() {
    const int W = cfg::RENDER_W, H = cfg::RENDER_H;
    DrawRectangle(0, 0, W, H, { 8, 4, 6, 140 });
    DrawCenteredText("BLOODRUSH", H / 2 - 130, 70, HUD_RED);
    DrawCenteredText("MANKIND IS DEAD. BLOOD IS FUEL.", H / 2 - 55, 16, HUD_DIM);
    if (fmodf((float)GetTime() * 1.6f, 1.0f) > 0.35f)
        DrawCenteredText("CLICK OR ENTER TO START", H / 2, 26, HUD_YELLOW);

    const char* lines[] = {
        "WASD + mouse  move    SPACE hold  bunny hop",
        "SHIFT  dash    CTRL  slide / air slam",
        "LMB  fire    RMB hold  charged shot    1/2  weapons",
        "Blood heals: deal damage up close",
    };
    for (int i = 0; i < 4; i++)
        DrawCenteredText(lines[i], H / 2 + 60 + i * 22, 16, HUD_WHITE);
    DrawCenteredText("Q — QUIT", H - 30, 16, HUD_DIM);
}

void DrawPauseOverlay() {
    const int W = cfg::RENDER_W, H = cfg::RENDER_H;
    DrawRectangle(0, 0, W, H, { 0, 0, 0, 160 });
    DrawCenteredText("PAUSED", H / 2 - 60, 50, HUD_RED);
    DrawCenteredText("ESC / CLICK — resume        Q — menu", H / 2 + 20, 18, HUD_WHITE);
}

void DrawDeathOverlay(const EnemyManager& enemies, const StyleMeter& style) {
    const int W = cfg::RENDER_W, H = cfg::RENDER_H;
    DrawRectangle(0, 0, W, H, { 40, 0, 4, 190 });
    DrawCenteredText("YOU DIED", H / 2 - 90, 60, HUD_RED);
    char buf[96];
    snprintf(buf, sizeof(buf), "WAVE %d      SCORE %ld", enemies.wave, style.score);
    DrawCenteredText(buf, H / 2 - 10, 26, HUD_WHITE);
    if (fmodf((float)GetTime() * 1.6f, 1.0f) > 0.35f)
        DrawCenteredText("CLICK — RETRY", H / 2 + 50, 22, HUD_YELLOW);
    DrawCenteredText("Q — MENU", H / 2 + 84, 16, HUD_DIM);
}

} // namespace

int main() {
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);
    InitWindow(1280, 720, "BLOODRUSH");
    SetExitKey(KEY_NULL);
    sfx::Init();

    RenderTexture2D target = LoadRenderTexture(cfg::RENDER_W, cfg::RENDER_H);
    SetTextureFilter(target.texture, TEXTURE_FILTER_POINT);
    Shader shading = LoadShadingShader();

    Arena arena;
    arena.Init();

    Player player;
    EnemyManager enemies;
    ParticleSystem particles;
    StyleMeter style;
    Weapons weapons;

    auto resetRun = [&]() {
        player.Init(arena.SpawnPoint());
        enemies.Reset();
        particles.Reset();
        style.Reset();
        weapons.Reset();
    };

    GameState state = GameState::Menu;
    float shakeTime = 0;
    bool quit = false;
    // dev/CI hook: aim at the nearest enemy automatically
    const bool autoAim = getenv("BLOODRUSH_AUTOTEST") != nullptr;

    while (!WindowShouldClose() && !quit) {
        float dt = fminf(GetFrameTime(), cfg::MAX_DT);
        if (IsKeyPressed(KEY_F11)) ToggleFullscreen();

        switch (state) {
            case GameState::Menu:
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || IsKeyPressed(KEY_ENTER)) {
                    resetRun();
                    state = GameState::Playing;
                    DisableCursor();
                    sfx::Play(sfx::CLICK);
                }
                if (IsKeyPressed(KEY_Q)) quit = true;
                break;

            case GameState::Playing: {
                if (IsKeyPressed(KEY_ESCAPE)) {
                    state = GameState::Paused;
                    EnableCursor();
                    break;
                }
                PlayerInput in = GatherInput();
                player.Update(in, arena, dt);
                if (autoAim) {
                    float best = 1e9f;
                    Vector3 eye = player.EyePos();
                    for (Enemy& e : enemies.All()) {
                        if (!e.alive || e.spawnT < 1) continue;
                        Vector3 d = Vector3Subtract(e.Center(), eye);
                        float len = Vector3Length(d);
                        if (len < best) {
                            best = len;
                            player.yaw = RAD2DEG * atan2f(d.x, -d.z);
                            player.pitch = RAD2DEG * asinf(d.y / len);
                        }
                    }
                }
                weapons.Update(player, arena, enemies, particles, style, dt);
                enemies.Update(player, arena, particles, style, dt);
                particles.Update(dt);
                style.Update(dt);
                shakeTime += dt * 40.0f;
                if (player.Dead()) {
                    state = GameState::Dead;
                    player.AddTrauma(1.0f);
                    EnableCursor();
                }
                break;
            }

            case GameState::Paused:
                if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_ENTER) ||
                    IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    state = GameState::Playing;
                    DisableCursor();
                }
                if (IsKeyPressed(KEY_Q)) state = GameState::Menu;
                break;

            case GameState::Dead:
                particles.Update(dt); // let the gore settle behind the overlay
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || IsKeyPressed(KEY_ENTER)) {
                    resetRun();
                    state = GameState::Playing;
                    DisableCursor();
                }
                if (IsKeyPressed(KEY_Q) || IsKeyPressed(KEY_ESCAPE))
                    state = GameState::Menu;
                break;
        }

        // Camera: gameplay cam or slow menu orbit
        Camera3D cam;
        if (state == GameState::Menu) {
            float t = (float)GetTime() * 0.15f;
            cam = {};
            cam.position = { sinf(t) * 48, 24, cosf(t) * 48 };
            cam.target = { 0, 4, 0 };
            cam.up = { 0, 1, 0 };
            cam.fovy = 70;
            cam.projection = CAMERA_PERSPECTIVE;
        } else {
            float sh = player.trauma * player.trauma;
            float shakeX = sh * 2.2f * sinf(shakeTime * 1.13f) * cosf(shakeTime * 0.71f);
            float shakeY = sh * 2.2f * sinf(shakeTime * 0.97f + 1.7f);
            cam = player.GetCamera(shakeX, shakeY);
        }

        // --- render into the low-res target ---
        BeginTextureMode(target);
        ClearBackground({ 8, 4, 6, 255 });
        BeginMode3D(cam);
        BeginShaderMode(shading);
        arena.Draw();
        enemies.Draw();
        weapons.Draw3D();
        particles.Draw();
        EndShaderMode();
        EndMode3D();

        if (state != GameState::Menu) {
            // viewmodel pass: depth test off so the gun never clips into walls
            BeginMode3D(cam);
            BeginShaderMode(shading);
            rlDrawRenderBatchActive();
            rlDisableDepthTest();
            weapons.DrawViewmodel(player);
            rlDrawRenderBatchActive();
            rlEnableDepthTest();
            EndShaderMode();
            EndMode3D();

            // damage / heal vignette
            if (player.hurtFlash > 0)
                DrawRectangle(0, 0, cfg::RENDER_W, cfg::RENDER_H,
                              Fade(HUD_RED, player.hurtFlash * 0.28f));

            DrawGameHud(player, enemies, style);
            style.Draw();
            weapons.DrawHUD();
        }

        if (state == GameState::Menu) DrawMenu();
        else if (state == GameState::Paused) DrawPauseOverlay();
        else if (state == GameState::Dead) DrawDeathOverlay(enemies, style);
        EndTextureMode();

        // --- upscale to the window ---
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

    UnloadShader(shading);
    UnloadRenderTexture(target);
    sfx::Shutdown();
    CloseWindow();
    return 0;
}
