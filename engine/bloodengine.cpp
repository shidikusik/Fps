#include "bloodengine.h"
#include "gamepad_db.h"
#include "settings.h"
#include "sounds.h"
#include "ui.h"

#include <string>

namespace be {

void Init(int winW, int winH, const char* title) {
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);
    InitWindow(winW, winH, title);
    SetExitKey(KEY_NULL);
    TraceLog(LOG_INFO, "%s v%s initialized", NAME, VERSION);

    // Load the embedded controller mapping database so gamepads map
    // correctly even when GLFW's built-in DB lacks the device (e.g. a
    // Bluetooth DualShock 4, whose GUID differs from the USB one).
    std::string db;
    db.reserve(1 << 19);
    for (int i = 0; i < GAMEPAD_DB_COUNT; i++) db += GAMEPAD_DB_LINES[i];
    SetGamepadMappings(db.c_str());

    sfx::Init();
    ui::Init();
    settings::Load();
}

void Shutdown() {
    ui::Shutdown();
    sfx::Shutdown();
    CloseWindow();
}

} // namespace be
