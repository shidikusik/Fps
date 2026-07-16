#include "localization.h"
#include "settings.h"

#include <cstdio>

namespace loc {

Lang Get() { return settings::Get().lang == 1 ? Lang::RU : Lang::EN; }

void Toggle() {
    settings::Get().lang = settings::Get().lang == 1 ? 0 : 1;
    settings::Save();
}

void LoadPref() { /* storage moved to settings::Load() */ }

const char* T(const char* en, const char* ru) {
    return Get() == Lang::RU ? ru : en;
}

const char* LevelName(int level) {
    if (Get() == Lang::RU) {
        switch (level) {
            case 2: return "КАТАКОМБЫ";
            case 3: return "АЛТАРЬ";
            case 4: return "ПЕЧЬ";
            case 5: return "ТРОН";
            default: return "ДВОР";
        }
    }
    switch (level) {
        case 2: return "THE CATACOMBS";
        case 3: return "THE ALTAR";
        case 4: return "THE FURNACE";
        case 5: return "THE THRONE";
        default: return "THE YARD";
    }
}

std::vector<std::string> IntroLines() {
    if (Get() == Lang::RU) return {
        "ЗЕМЛЯ МОЛЧИТ.",
        "МАШИНА ПРОБУЖДАЕТСЯ.",
        "ТОПЛИВО: НОЛЬ.",
        "НАЙДЕН ИСТОЧНИК: КРОВЬ.",
        "БАШНЯ ЗОВЁТ. СПУСКАЙСЯ.",
    };
    return {
        "EARTH IS SILENT.",
        "THE MACHINE REACTIVATES.",
        "FUEL RESERVES: EMPTY.",
        "ALTERNATIVE SOURCE LOCATED: BLOOD.",
        "THE TOWER CALLS. DESCEND.",
    };
}

std::vector<std::string> LevelLines(int level) {
    if (Get() == Lang::RU) {
        switch (level) {
            case 2: return {
                "СЛОЙ ЗАЧИЩЕН.",
                "ПОД ДВОРОМ: КАТАКОМБЫ.",
                "МЁРТВЫЕ ЗДЕСЬ НЕ СПЯТ.",
                "ОНИ ЗНАЮТ, ЧТО ТЫ ИДЁШЬ.",
            };
            case 3: return {
                "КАТАКОМБЫ ЗАТИХЛИ.",
                "НИЖЕ: АЛТАРЬ.",
                "ЗДЕСЬ МОЛИЛИСЬ НЕ ЛЮДИ.",
                "ГЛУБЖЕ ЕСТЬ ЕЩЁ.",
            };
            case 4: return {
                "АЛТАРЬ ОСЫПАЕТСЯ.",
                "СНИЗУ ЖАР: ПЕЧЬ.",
                "СТАЛЬ ПЛАВИТСЯ.",
                "ТЫ — НЕТ.",
            };
            default: return {
                "ПЕЧЬ УГАСАЕТ.",
                "НА ДНЕ: ТРОН.",
                "ХРАНИТЕЛЬ ЖДЁТ.",
                "ЗАКОНЧИ ЭТО.",
            };
        }
    }
    switch (level) {
        case 2: return {
            "LAYER CLEARED.",
            "BELOW THE YARD: THE CATACOMBS.",
            "THE DEAD HERE DO NOT REST.",
            "THEY KNOW YOU ARE COMING.",
        };
        case 3: return {
            "THE CATACOMBS FALL SILENT.",
            "BELOW: THE ALTAR.",
            "WHAT PRAYED HERE WAS NOT HUMAN.",
            "THERE IS MORE BENEATH.",
        };
        case 4: return {
            "THE ALTAR CRUMBLES.",
            "HEAT RISES: THE FURNACE.",
            "STEEL MELTS HERE.",
            "YOU DO NOT.",
        };
        default: return {
            "THE FURNACE DIES OUT.",
            "AT THE BOTTOM: THE THRONE.",
            "THE WARDEN IS WAITING.",
            "END IT.",
        };
    }
}

std::vector<std::string> VictoryLines(int nextLoop) {
    char buf[96];
    if (Get() == Lang::RU) {
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
