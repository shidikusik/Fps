#include "localization.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace loc {
namespace {

Lang lang = Lang::EN;

std::string PrefPath() {
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
    if (dir.empty()) return ""; // e.g. Android: session-only choice
    return dir + "/bloodrush.lang";
}

void Save() {
    std::string p = PrefPath();
    if (p.empty()) return;
    FILE* f = fopen(p.c_str(), "w");
    if (f) {
        fputs(lang == Lang::RU ? "ru" : "en", f);
        fclose(f);
    }
}

} // namespace

Lang Get() { return lang; }

void Toggle() {
    lang = lang == Lang::RU ? Lang::EN : Lang::RU;
    Save();
}

void LoadPref() {
    std::string p = PrefPath();
    if (p.empty()) return;
    FILE* f = fopen(p.c_str(), "r");
    if (!f) return;
    char buf[8] = {};
    fread(buf, 1, sizeof(buf) - 1, f);
    fclose(f);
    if (strncmp(buf, "ru", 2) == 0) lang = Lang::RU;
}

const char* T(const char* en, const char* ru) {
    return lang == Lang::RU ? ru : en;
}

const char* LevelName(int level) {
    if (lang == Lang::RU) {
        switch (level) {
            case 2: return "КАТАКОМБЫ";
            case 3: return "АЛТАРЬ";
            default: return "ДВОР";
        }
    }
    switch (level) {
        case 2: return "THE CATACOMBS";
        case 3: return "THE ALTAR";
        default: return "THE YARD";
    }
}

std::vector<std::string> IntroLines() {
    if (lang == Lang::RU) return {
        "ЗЕМЛЯ МОЛЧИТ.",
        "БОЕВАЯ МАШИНА V-13 ПРОБУЖДАЕТСЯ.",
        "ТОПЛИВО: НОЛЬ.",
        "НАЙДЕН ИСТОЧНИК: КРОВЬ.",
        "БАШНЯ ЗОВЁТ. СПУСКАЙСЯ.",
    };
    return {
        "EARTH IS SILENT.",
        "MACHINE UNIT V-13 REACTIVATES.",
        "FUEL RESERVES: EMPTY.",
        "ALTERNATIVE SOURCE LOCATED: BLOOD.",
        "THE TOWER CALLS. DESCEND.",
    };
}

std::vector<std::string> LevelLines(int level) {
    if (lang == Lang::RU) {
        if (level == 2) return {
            "СЛОЙ ЗАЧИЩЕН.",
            "ПОД ДВОРОМ: КАТАКОМБЫ.",
            "МЁРТВЫЕ ЗДЕСЬ НЕ СПЯТ.",
            "ОНИ ЗНАЮТ, ЧТО ТЫ ИДЁШЬ.",
        };
        return {
            "КАТАКОМБЫ ЗАТИХЛИ.",
            "ОСТАЛСЯ ОДИН СЛОЙ: АЛТАРЬ.",
            "ЕГО ХРАНИТ НЕЧТО ДРЕВНЕЕ.",
            "ХРАНИТЕЛЬ ПРОСЫПАЕТСЯ.",
        };
    }
    if (level == 2) return {
        "LAYER CLEARED.",
        "BELOW THE YARD: THE CATACOMBS.",
        "THE DEAD HERE DO NOT REST.",
        "THEY KNOW YOU ARE COMING.",
    };
    return {
        "THE CATACOMBS FALL SILENT.",
        "ONE LAYER REMAINS: THE ALTAR.",
        "SOMETHING ANCIENT GUARDS IT.",
        "THE WARDEN STIRS.",
    };
}

std::vector<std::string> VictoryLines(int nextLoop) {
    char buf[96];
    if (lang == Lang::RU) {
        snprintf(buf, sizeof(buf), "КРУГ %d. КРОМЕ НАСИЛИЯ НЕТ НИЧЕГО.", nextLoop + 1);
        return {
            "ХРАНИТЕЛЬ ПАЛ.",
            "БАШНЯ НЕ ВЫПУСКАЕТ.",
            "ТОЛЬКО ГЛУБЖЕ.",
            std::string(buf),
        };
    }
    snprintf(buf, sizeof(buf), "LOOP %d. NOTHING LEFT BUT VIOLENCE.", nextLoop + 1);
    return {
        "THE WARDEN FALLS.",
        "THE TOWER OFFERS NO EXIT.",
        "ONLY DEEPER.",
        std::string(buf),
    };
}

} // namespace loc
