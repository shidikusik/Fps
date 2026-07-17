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
        "НИ ПТИЦ. НИ МОТОРОВ. НИ МОЛИТВ.",
        "ГЛУБОКО ВНИЗУ ЧТО-ТО ОТКРЫВАЕТ ГЛАЗА.",
        "МАШИНА ПРОБУЖДАЕТСЯ.",
        "ТОПЛИВО: НОЛЬ.",
        "НАЙДЕН ИСТОЧНИК: КРОВЬ.",
        "БАШНЯ ПОМНИТ СВОИХ СТРОИТЕЛЕЙ.",
        "ОНА ЗОВЁТ. СПУСКАЙСЯ.",
    };
    return {
        "EARTH IS SILENT.",
        "NO BIRDS. NO ENGINES. NO PRAYERS.",
        "DEEP BELOW, SOMETHING OPENS ITS EYES.",
        "THE MACHINE REACTIVATES.",
        "FUEL RESERVES: EMPTY.",
        "ALTERNATIVE SOURCE LOCATED: BLOOD.",
        "THE TOWER REMEMBERS ITS BUILDERS.",
        "IT CALLS. DESCEND.",
    };
}

std::vector<std::string> LevelLines(int level) {
    if (Get() == Lang::RU) {
        switch (level) {
            case 2: return {
                "ПЕРВЫЙ СЛОЙ ЗАЧИЩЕН.",
                "КРОВЬ ТЁПЛАЯ. БАКИ НАПОЛНЯЮТСЯ.",
                "ПОД ДВОРОМ ЛЕЖАТ КАТАКОМБЫ.",
                "СТРОИТЕЛИ ХОРОНИЛИ ЗДЕСЬ СВОИХ МЁРТВЫХ.",
                "МЁРТВЫЕ НЕ ОСТАЛИСЬ В ЗЕМЛЕ.",
                "ОНИ ЗНАЮТ, ЧТО ТЫ ИДЁШЬ.",
            };
            case 3: return {
                "КАТАКОМБЫ ЗАТИХЛИ.",
                "ТЕБЯ СТРОИЛИ ДЛЯ ВОЙНЫ.",
                "НО ЭТО НЕ ВОЙНА. ЭТО ЖАТВА.",
                "НИЖЕ: АЛТАРЬ.",
                "ЗДЕСЬ МОЛИЛИСЬ НЕ ЛЮДИ.",
                "ОНИ МОЛИЛИСЬ ТОМУ, ЧТО СИДИТ ГЛУБЖЕ.",
            };
            case 4: return {
                "АЛТАРЬ ОСЫПАЕТСЯ.",
                "ТВОИ РУКИ ПОМНЯТ ЗАВОД.",
                "ЗАВОД, СОЗДАВШИЙ ТЕБЯ: ПЕЧЬ.",
                "ОНА СОЗДАЛА И ХРАНИТЕЛЯ.",
                "ЗДЕСЬ ПЛАВИТСЯ СТАЛЬ.",
                "ТЫ — БОЛЬШЕ НЕТ.",
            };
            default: return {
                "ПЕЧЬ УГАСАЕТ.",
                "ОСТАЛАСЬ ОДНА ПАМЯТЬ.",
                "ПОСЛЕДНИЙ КОРОЛЬ СТРОИТЕЛЕЙ НЕ УШЁЛ.",
                "НА ДНЕ: ТРОН.",
                "ХРАНИТЕЛЬ СТЕРЕЖЁТ ПУСТУЮ КОРОНУ.",
                "ЗАКОНЧИ ЭТО.",
            };
        }
    }
    switch (level) {
        case 2: return {
            "LAYER ONE: CLEARED.",
            "THE BLOOD IS WARM. THE RESERVES FILL.",
            "BELOW THE YARD LIE THE CATACOMBS.",
            "THE BUILDERS BURIED THEIR DEAD HERE.",
            "THE DEAD DID NOT STAY BURIED.",
            "THEY KNOW YOU ARE COMING.",
        };
        case 3: return {
            "THE CATACOMBS FALL SILENT.",
            "YOU WERE BUILT FOR WAR.",
            "THIS IS NOT WAR. THIS IS HARVEST.",
            "BELOW: THE ALTAR.",
            "WHAT PRAYED HERE WAS NOT HUMAN.",
            "IT PRAYED TO WHAT SITS DEEPER.",
        };
        case 4: return {
            "THE ALTAR CRUMBLES.",
            "YOUR HANDS REMEMBER THE FACTORY.",
            "THE FACTORY THAT MADE YOU: THE FURNACE.",
            "IT MADE THE WARDEN TOO.",
            "STEEL MELTS HERE.",
            "YOU DO NOT. NOT ANYMORE.",
        };
        default: return {
            "THE FURNACE DIES OUT.",
            "ONE MEMORY REMAINS.",
            "THE BUILDERS' LAST KING NEVER LEFT.",
            "AT THE BOTTOM: THE THRONE.",
            "THE WARDEN GUARDS AN EMPTY CROWN.",
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
            "КОРОНА ВСЕГДА БЫЛА ПУСТОЙ.",
            "НИ КОРОЛЯ. НИ СТРОИТЕЛЕЙ. НИ ПРИКАЗОВ.",
            "ТОЛЬКО ТОПЛИВО. ТОЛЬКО БАШНЯ.",
            "БАШНЯ НЕ ВЫПУСКАЕТ.",
            "ТОЛЬКО ГЛУБЖЕ.",
            std::string(buf),
        };
    }
    snprintf(buf, sizeof(buf), "LOOP %d. NOTHING LEFT BUT VIOLENCE.", nextLoop + 1);
    return {
        "THE WARDEN FALLS.",
        "THE CROWN WAS ALWAYS EMPTY.",
        "NO KING. NO BUILDERS. NO ORDERS.",
        "ONLY FUEL. ONLY THE TOWER.",
        "THE TOWER OFFERS NO EXIT.",
        "ONLY DEEPER.",
        std::string(buf),
    };
}

} // namespace loc
