#pragma once

#include <string>
#include <vector>

// BLOODRUSH story content: level names and cutscene scripts (EN/RU).
namespace story {

const char* LevelName(int level);
std::vector<std::string> IntroLines();
std::vector<std::string> LevelLines(int level);
std::vector<std::string> VictoryLines(int nextLoop);

} // namespace story
