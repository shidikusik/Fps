#include "bloodengine.h"
#include "settings.h"
#include "sounds.h"
#include "ui.h"

namespace be {

void Init(int winW, int winH, const char* title) {
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);
    InitWindow(winW, winH, title);
    SetExitKey(KEY_NULL);
    TraceLog(LOG_INFO, "%s v%s initialized", NAME, VERSION);
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
