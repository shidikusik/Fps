// BLOODRUSH — fast-paced arena FPS.
// Three levels, story cutscenes, four weapons, a boss, endless NG+ loops.

#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

#include "arena.h"
#include "config.h"
#include "cutscene.h"
#include "enemies.h"
#include "localization.h"
#include "particles.h"
#include "player.h"
#include "shading.h"
#include "sounds.h"
#include "style_meter.h"
#include "touch.h"
#include "ui.h"
#include "weapons.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

namespace {

constexpr Color HUD_RED    = { 230, 30, 40, 255 };
constexpr Color HUD_YELLOW = { 255, 230, 0, 255 };
constexpr Color HUD_WHITE  = { 235, 230, 230, 255 };
constexpr Color HUD_DIM    = { 120, 60, 64, 255 };

enum class GameState { Menu, Cutscene, Playing, Paused, Dead };
enum class AfterCutscene { BeginRun, ResumePlay };

Rectangle LangButtonRect() {
    const char* label = loc::T("LANGUAGE: ENGLISH [L]", "ЯЗЫК: РУССКИЙ [L]");
    float w = (float)ui::Measure(label, 14) + 24;
    return { 16, (float)cfg::RENDER_H - 46, w, 30 };
}

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

CombatInput GatherCombatInput(const PlayerInput& pin) {
    CombatInput in;
    in.fireHeld = IsMouseButtonDown(MOUSE_BUTTON_LEFT);
    in.altHeld = IsMouseButtonDown(MOUSE_BUTTON_RIGHT);
    if (IsKeyPressed(KEY_ONE)) in.select = 0;
    if (IsKeyPressed(KEY_TWO)) in.select = 1;
    if (IsKeyPressed(KEY_THREE)) in.select = 2;
    if (IsKeyPressed(KEY_FOUR)) in.select = 3;
    float wheel = GetMouseWheelMove();
    in.cycle = wheel > 0 ? 1 : (wheel < 0 ? -1 : 0);
    in.lookDx = pin.mouseDx;
    in.lookDy = pin.mouseDy;
    return in;
}

void DrawCenteredText(const char* text, int y, int size, Color col) {
    ui::TextCentered(text, y, size, col);
}

void DrawSky() {
    // deep gradient behind everything
    DrawRectangleGradientV(0, 0, cfg::RENDER_W, cfg::RENDER_H,
                           { 16, 4, 10, 255 }, { 4, 2, 6, 255 });
}

void DrawGameHud(const Player& player, const EnemyManager& enemies,
                 const StyleMeter& style, const Arena& arena) {
    const int W = cfg::RENDER_W, H = cfg::RENDER_H;

    // Crosshair
    DrawLine(W / 2 - 6, H / 2, W / 2 - 2, H / 2, HUD_WHITE);
    DrawLine(W / 2 + 2, H / 2, W / 2 + 6, H / 2, HUD_WHITE);
    DrawLine(W / 2, H / 2 - 6, W / 2, H / 2 - 2, HUD_WHITE);
    DrawLine(W / 2, H / 2 + 2, W / 2, H / 2 + 6, HUD_WHITE);

    // Speed (bottom left)
    float speed = player.HorizontalSpeed();
    char buf[96];
    snprintf(buf, sizeof(buf), "%5.1f", speed);
    Color speedCol = HUD_WHITE;
    if (speed > cfg::WALK_SPEED * 1.2f) speedCol = HUD_YELLOW;
    if (speed > cfg::DASH_SPEED * 0.8f) speedCol = HUD_RED;
    ui::Text(buf, 16, H - 58, 32, speedCol);
    ui::Text("UPS", 16 + ui::Measure(buf, 32) + 8, H - 42, 16, HUD_DIM);
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
    if (state) ui::Text(state, 16, H - 108, 16, HUD_RED);

    // Health (bottom center)
    int hp = (int)ceilf(player.hp);
    Color hpCol = hp > 30 ? HUD_RED : Color{ 255, 60, 30, 255 };
    float pulse = hp <= 30 ? (0.5f + 0.5f * sinf((float)GetTime() * 9)) : 1.0f;
    snprintf(buf, sizeof(buf), "%d", hp < 0 ? 0 : hp);
    int bx = W / 2 - 110;
    ui::Text(buf, bx - ui::Measure(buf, 28) - 10, H - 44, 28, Fade(hpCol, pulse));
    DrawRectangle(bx, H - 34, 220, 12, { 40, 20, 22, 255 });
    DrawRectangle(bx, H - 34, (int)(220 * Clamp(player.hp / player.maxHp, 0.0f, 1.0f)),
                  12, Fade(hpCol, pulse));
    DrawRectangleLines(bx - 1, H - 35, 222, 14, HUD_DIM);
    if (player.healFlash > 0)
        ui::Text(loc::T("+BLOOD", "+КРОВЬ"), bx + 228, H - 38, 14,
                 Fade(HUD_YELLOW, player.healFlash));

    // Level / wave / score (top)
    if (enemies.waveActive) {
        snprintf(buf, sizeof(buf),
                 loc::T("%s — WAVE %d/%d — %d LEFT", "%s — ВОЛНА %d/%d — ОСТАЛОСЬ %d"),
                 loc::LevelName(arena.Level()), enemies.waveInLevel,
                 EnemyManager::WAVES_PER_LEVEL, enemies.AliveCount());
        DrawCenteredText(buf, 10, 16, HUD_WHITE);
    } else if (!enemies.levelCleared) {
        snprintf(buf, sizeof(buf), loc::T("WAVE %d IN %d", "ВОЛНА %d ЧЕРЕЗ %d"),
                 enemies.waveInLevel + 1, (int)ceilf(enemies.intermission));
        DrawCenteredText(buf, 10, 22, HUD_YELLOW);
        if (enemies.waveInLevel > 0)
            DrawCenteredText(loc::T("WAVE CLEARED", "ВОЛНА ЗАЧИЩЕНА"), 40, 14, HUD_DIM);
    }
    if (enemies.loop > 0) {
        snprintf(buf, sizeof(buf), loc::T("LOOP %d", "КРУГ %d"), enemies.loop + 1);
        ui::Text(buf, 16, 10, 16, HUD_RED);
    }
    snprintf(buf, sizeof(buf), "%ld", style.score);
    ui::Text(buf, W - ui::Measure(buf, 22) - 16, 10, 22, HUD_YELLOW);

    // Boss bar
    if (const Enemy* boss = enemies.Boss()) {
        float bt = Clamp(boss->hp / boss->maxHp, 0.0f, 1.0f);
        int bw = 420, bx2 = W / 2 - bw / 2, by = 44;
        DrawCenteredText(loc::T("THE WARDEN", "ХРАНИТЕЛЬ"), by - 4, 16,
                         { 255, 220, 120, 255 });
        DrawRectangle(bx2, by + 16, bw, 10, { 40, 20, 22, 255 });
        DrawRectangle(bx2, by + 16, (int)(bw * bt), 10, HUD_RED);
        DrawRectangleLines(bx2 - 1, by + 15, bw + 2, 12, { 255, 220, 120, 255 });
    }

    DrawFPS(8, H - 26);
}

void DrawMenu() {
    const int W = cfg::RENDER_W, H = cfg::RENDER_H;
    DrawRectangle(0, 0, W, H, { 8, 4, 6, 140 });
    DrawCenteredText("BLOODRUSH", H / 2 - 160, 72, HUD_RED);
    DrawCenteredText(loc::T("MANKIND IS DEAD. BLOOD IS FUEL.",
                            "ЧЕЛОВЕЧЕСТВО МЕРТВО. КРОВЬ — ТОПЛИВО."),
                     H / 2 - 72, 16, HUD_DIM);
    if (fmodf((float)GetTime() * 1.6f, 1.0f) > 0.35f)
        DrawCenteredText(loc::T("CLICK OR ENTER TO START", "КЛИК ИЛИ ENTER — СТАРТ"),
                         H / 2 - 14, 24, HUD_YELLOW);

    const char* linesEn[] = {
        "WASD + mouse  move    SPACE hold  bunny hop",
        "SHIFT  dash    CTRL  slide / air slam",
        "LMB fire   RMB hold  charged shot   1-4 weapons",
        "Blood heals: deal damage up close",
        "3 layers. 4 waves each. The Warden waits below.",
    };
    const char* linesRu[] = {
        "WASD + мышь  движение    SPACE держать  bhop",
        "SHIFT  рывок    CTRL  подкат / удар вниз",
        "ЛКМ огонь   ПКМ держать  заряж. выстрел   1-4 оружие",
        "Кровь лечит: бей врагов в упор",
        "3 слоя по 4 волны. Внизу ждёт Хранитель.",
    };
    for (int i = 0; i < 5; i++)
        DrawCenteredText(loc::T(linesEn[i], linesRu[i]), H / 2 + 56 + i * 26, 14,
                         HUD_WHITE);
    DrawCenteredText(loc::T("Q — QUIT", "Q — ВЫХОД"), H - 34, 14, HUD_DIM);

    // language toggle button
    Rectangle lb = LangButtonRect();
    DrawRectangleRec(lb, Fade(BLACK, 0.4f));
    DrawRectangleLinesEx(lb, 1, HUD_DIM);
    ui::Text(loc::T("LANGUAGE: ENGLISH [L]", "ЯЗЫК: РУССКИЙ [L]"),
             (int)lb.x + 12, (int)lb.y + 8, 14, HUD_WHITE);
}

void DrawPauseOverlay() {
    const int W = cfg::RENDER_W, H = cfg::RENDER_H;
    DrawRectangle(0, 0, W, H, { 0, 0, 0, 160 });
    DrawCenteredText(loc::T("PAUSED", "ПАУЗА"), H / 2 - 60, 44, HUD_RED);
    DrawCenteredText(loc::T("ESC / CLICK — resume        Q — menu",
                            "ESC / КЛИК — продолжить        Q — меню"),
                     H / 2 + 20, 15, HUD_WHITE);
}

void DrawDeathOverlay(const EnemyManager& enemies, const StyleMeter& style,
                      const Arena& arena) {
    const int W = cfg::RENDER_W, H = cfg::RENDER_H;
    DrawRectangle(0, 0, W, H, { 40, 0, 4, 190 });
    DrawCenteredText(loc::T("YOU DIED", "ТЫ ПОГИБ"), H / 2 - 100, 56, HUD_RED);
    char buf[192];
    if (enemies.loop > 0)
        snprintf(buf, sizeof(buf),
                 loc::T("%s — WAVE %d — LOOP %d — SCORE %ld",
                        "%s — ВОЛНА %d — КРУГ %d — СЧЁТ %ld"),
                 loc::LevelName(arena.Level()), enemies.waveInLevel,
                 enemies.loop + 1, style.score);
    else
        snprintf(buf, sizeof(buf),
                 loc::T("%s — WAVE %d — SCORE %ld", "%s — ВОЛНА %d — СЧЁТ %ld"),
                 loc::LevelName(arena.Level()), enemies.waveInLevel, style.score);
    DrawCenteredText(buf, H / 2 - 10, 18, HUD_WHITE);
    if (fmodf((float)GetTime() * 1.6f, 1.0f) > 0.35f)
        DrawCenteredText(loc::T("CLICK / ENTER — RETRY", "КЛИК / ENTER — ЗАНОВО"),
                         H / 2 + 50, 20, HUD_YELLOW);
    DrawCenteredText(loc::T("Q — MENU", "Q — МЕНЮ"), H / 2 + 86, 14, HUD_DIM);
}

Camera3D CinematicCamera(float t01, int level) {
    // slow dolly-in during cutscenes / menu orbit
    Camera3D cam{};
    float ang = 0.6f + t01 * 0.9f;
    float rad = 58.0f - t01 * 22.0f;
    float height = 26.0f - t01 * 8.0f + level * 2.0f;
    cam.position = { sinf(ang) * rad, height, cosf(ang) * rad };
    cam.target = { 0, 5.0f + level, 0 };
    cam.up = { 0, 1, 0 };
    cam.fovy = 66;
    cam.projection = CAMERA_PERSPECTIVE;
    return cam;
}

} // namespace

int main() {
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);
    InitWindow(1280, 720, "BLOODRUSH");
    SetExitKey(KEY_NULL);
    sfx::Init();
    ui::Init();
    loc::LoadPref();

    RenderTexture2D target = LoadRenderTexture(cfg::RENDER_W, cfg::RENDER_H);
    SetTextureFilter(target.texture, TEXTURE_FILTER_POINT);
    Shader shading = LoadShadingShader();

    Arena arena;
    arena.Init(1);

    Player player;
    EnemyManager enemies;
    ParticleSystem particles;
    StyleMeter style;
    Weapons weapons;
    Cutscene cutscene;
    AfterCutscene afterCutscene = AfterCutscene::BeginRun;

    auto resetRun = [&]() {
        arena.Init(1);
        player.Init(arena.SpawnPoint());
        enemies.BeginLevel(1, 0);
        particles.Reset();
        style.Reset();
        weapons.Reset();
    };

    GameState state = GameState::Menu;
    float shakeTime = 0;
    bool quit = false;
    const bool autoAim = getenv("BLOODRUSH_AUTOTEST") != nullptr;
#ifdef __ANDROID__
    const bool touchUI = true;
#else
    const bool touchUI = getenv("BLOODRUSH_TOUCH") != nullptr;
#endif
    TouchControls touch;

    while (!WindowShouldClose() && !quit) {
        float dt = fminf(GetFrameTime(), cfg::MAX_DT);
        if (IsKeyPressed(KEY_F11)) ToggleFullscreen();

        // letterbox mapping for this frame (used by touch input + final blit)
        RenderMap map;
        map.scale = fminf((float)GetScreenWidth() / cfg::RENDER_W,
                          (float)GetScreenHeight() / cfg::RENDER_H);
        map.offX = (GetScreenWidth() - cfg::RENDER_W * map.scale) * 0.5f;
        map.offY = (GetScreenHeight() - cfg::RENDER_H * map.scale) * 0.5f;

        switch (state) {
            case GameState::Menu: {
                bool langClicked = false;
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    Vector2 mp = map.ToRender(GetMousePosition());
                    if (CheckCollisionPointRec(mp, LangButtonRect())) {
                        langClicked = true;
                        loc::Toggle();
                        sfx::Play(sfx::CLICK, 0.6f);
                    }
                }
                if (IsKeyPressed(KEY_L)) {
                    loc::Toggle();
                    sfx::Play(sfx::CLICK, 0.6f);
                }
                if ((IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !langClicked) ||
                    IsKeyPressed(KEY_ENTER)) {
                    resetRun();
                    cutscene.Start(loc::IntroLines());
                    afterCutscene = AfterCutscene::BeginRun;
                    state = GameState::Cutscene;
                    sfx::Play(sfx::CLICK);
                }
                if (IsKeyPressed(KEY_Q)) quit = true;
                break;
            }

            case GameState::Cutscene:
                cutscene.Update(dt);
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || IsKeyPressed(KEY_ENTER) ||
                    IsKeyPressed(KEY_ESCAPE))
                    cutscene.Skip();
                if (cutscene.Finished()) {
                    state = GameState::Playing;
                    DisableCursor();
                }
                break;

            case GameState::Playing: {
                PlayerInput in;
                CombatInput cin;
                bool pausePressed = IsKeyPressed(KEY_ESCAPE) ||
                                    IsKeyPressed(KEY_BACK);
                if (touchUI) {
                    bool touchPause = false;
                    touch.Gather(map, in, cin, touchPause);
                    pausePressed = pausePressed || touchPause;
                } else {
                    in = GatherInput();
                    cin = GatherCombatInput(in);
                }
                if (pausePressed) {
                    state = GameState::Paused;
                    EnableCursor();
                    break;
                }
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
                weapons.Update(player, arena, enemies, particles, style, cin, dt);
                enemies.Update(player, arena, particles, style, dt);
                particles.Update(dt);
                style.Update(dt);
                shakeTime += dt * 40.0f;

                // level progression
                if (enemies.levelCleared) {
                    if (arena.Level() < Arena::NUM_LEVELS) {
                        int next = arena.Level() + 1;
                        arena.Init(next);
                        enemies.BeginLevel(next, enemies.loop);
                        player.pos = arena.SpawnPoint();
                        player.vel = { 0, 0, 0 };
                        player.Heal(50);
                        particles.Reset();
                        cutscene.Start(loc::LevelLines(next));
                    } else {
                        // Warden down: victory, then loop deeper
                        int nextLoop = enemies.loop + 1;
                        style.score += 2000L * nextLoop;
                        arena.Init(1);
                        enemies.BeginLevel(1, nextLoop);
                        player.pos = arena.SpawnPoint();
                        player.vel = { 0, 0, 0 };
                        player.hp = player.maxHp;
                        particles.Reset();
                        cutscene.Start(loc::VictoryLines(enemies.loop));
                    }
                    afterCutscene = AfterCutscene::ResumePlay;
                    state = GameState::Cutscene;
                    EnableCursor();
                    break;
                }

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
                particles.Update(dt);
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || IsKeyPressed(KEY_ENTER)) {
                    resetRun();
                    state = GameState::Playing;
                    DisableCursor();
                }
                if (IsKeyPressed(KEY_Q) || IsKeyPressed(KEY_ESCAPE))
                    state = GameState::Menu;
                break;
        }

        // Camera
        Camera3D cam;
        if (state == GameState::Menu) {
            cam = CinematicCamera(fmodf((float)GetTime() * 0.02f, 1.0f), arena.Level());
        } else if (state == GameState::Cutscene) {
            cam = CinematicCamera(cutscene.Progress(), arena.Level());
        } else {
            float sh = player.trauma * player.trauma;
            float shakeX = sh * 2.2f * sinf(shakeTime * 1.13f) * cosf(shakeTime * 0.71f);
            float shakeY = sh * 2.2f * sinf(shakeTime * 0.97f + 1.7f);
            cam = player.GetCamera(shakeX, shakeY);
        }

        // --- render into the target ---
        BeginTextureMode(target);
        ClearBackground({ 8, 4, 6, 255 });
        DrawSky();
        BeginMode3D(cam);
        BeginShaderMode(shading);
        arena.Draw();
        enemies.Draw();
        weapons.Draw3D();
        particles.Draw();
        EndShaderMode();
        EndMode3D();

        bool hudStates = state == GameState::Playing || state == GameState::Paused ||
                         state == GameState::Dead;
        if (hudStates) {
            BeginMode3D(cam);
            BeginShaderMode(shading);
            rlDrawRenderBatchActive();
            rlDisableDepthTest();
            weapons.DrawViewmodel(player);
            rlDrawRenderBatchActive();
            rlEnableDepthTest();
            EndShaderMode();
            EndMode3D();

            if (player.hurtFlash > 0)
                DrawRectangle(0, 0, cfg::RENDER_W, cfg::RENDER_H,
                              Fade(HUD_RED, player.hurtFlash * 0.28f));

            DrawGameHud(player, enemies, style, arena);
            style.Draw();
            weapons.DrawHUD();
            if (touchUI && state == GameState::Playing) touch.Draw();
        }

        if (state == GameState::Menu) DrawMenu();
        else if (state == GameState::Cutscene) cutscene.Draw();
        else if (state == GameState::Paused) DrawPauseOverlay();
        else if (state == GameState::Dead) DrawDeathOverlay(enemies, style, arena);
        EndTextureMode();

        // --- upscale to the window ---
        BeginDrawing();
        ClearBackground(BLACK);
        Rectangle src = { 0, 0, (float)cfg::RENDER_W, -(float)cfg::RENDER_H };
        Rectangle dst = { map.offX, map.offY,
                          cfg::RENDER_W * map.scale, cfg::RENDER_H * map.scale };
        DrawTexturePro(target.texture, src, dst, { 0, 0 }, 0, WHITE);
        EndDrawing();
    }

    UnloadShader(shading);
    UnloadRenderTexture(target);
    ui::Shutdown();
    sfx::Shutdown();
    CloseWindow();
    return 0;
}
