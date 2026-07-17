#include "settings.h"
#include "rumble.h"
#include "raylib.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

namespace settings {

std::string ConfigDir() {
    std::string dir;
    const char* xdg = getenv("XDG_CONFIG_HOME");
    if (xdg && *xdg) dir = xdg;
    else {
        const char* home = getenv("HOME");
        if (home && *home) dir = std::string(home) + "/.config";
    }
#ifdef _WIN32
    if (dir.empty()) {
        const char* appdata = getenv("APPDATA");
        if (appdata && *appdata) dir = appdata;
    }
#endif
    return dir; // "" e.g. on Android: session-only settings
}

namespace {

Values values;

std::string Path() {
    std::string dir = ConfigDir();
    if (dir.empty()) return "";
    return dir + "/bloodrush.cfg";
}

float ClampF(float v, float lo, float hi) { return v < lo ? lo : (v > hi ? hi : v); }

} // namespace

Values& Get() { return values; }

void ApplyVolume() { SetMasterVolume(values.volume); }

void Save() {
    std::string p = Path();
    if (p.empty()) return;
    FILE* f = fopen(p.c_str(), "w");
    if (!f) return;
    fprintf(f, "lang=%s\nsens=%.2f\nvol=%.2f\nvib=%d\n",
            values.lang == 1 ? "ru" : "en", values.sensitivity, values.volume,
            values.vibration ? 1 : 0);
    fclose(f);
}

void Load() {
    std::string p = Path();
    if (!p.empty()) {
        FILE* f = fopen(p.c_str(), "r");
        if (f) {
            char line[64];
            while (fgets(line, sizeof(line), f)) {
                if (strncmp(line, "lang=ru", 7) == 0) values.lang = 1;
                else if (strncmp(line, "lang=en", 7) == 0) values.lang = 0;
                else if (strncmp(line, "sens=", 5) == 0)
                    values.sensitivity = ClampF((float)atof(line + 5), 0.4f, 2.0f);
                else if (strncmp(line, "vol=", 4) == 0)
                    values.volume = ClampF((float)atof(line + 4), 0.0f, 1.0f);
                else if (strncmp(line, "vib=", 4) == 0)
                    values.vibration = line[4] != '0';
            }
            fclose(f);
        }
    }
    ApplyVolume();
    rumble::SetEnabled(values.vibration);
}

} // namespace settings
