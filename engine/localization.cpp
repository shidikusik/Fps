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

} // namespace loc
