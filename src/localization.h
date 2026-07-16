#pragma once

#include <string>
#include <vector>

enum class Lang { EN = 0, RU = 1 };

// Tiny two-language string system. UI text passes both variants at the
// call site via T(); story lines and level names live in localization.cpp.
// The choice persists in the user's config directory (where available).
namespace loc {

Lang Get();
void Toggle();     // switches language and saves the preference
void LoadPref();   // call once at startup

const char* T(const char* en, const char* ru);

const char* LevelName(int level);
std::vector<std::string> IntroLines();
std::vector<std::string> LevelLines(int level);
std::vector<std::string> VictoryLines(int nextLoop);

} // namespace loc
