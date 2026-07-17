#include "savegame.h"
#include "settings.h"

#include <cstdio>
#include <string>

namespace save {
namespace {
std::string Path() {
    std::string dir = settings::ConfigDir();
    if (dir.empty()) return "";
    return dir + "/bloodrush.save";
}
} // namespace

void Write(const Data& d) {
    std::string p = Path();
    if (p.empty()) return;
    FILE* f = fopen(p.c_str(), "w");
    if (!f) return;
    fprintf(f, "level=%d\nloop=%d\nscore=%ld\nhp=%.1f\nweapon=%d\n",
            d.level, d.loop, d.score, d.hp, d.weapon);
    fclose(f);
}

bool Read(Data& d) {
    std::string p = Path();
    if (p.empty()) return false;
    FILE* f = fopen(p.c_str(), "r");
    if (!f) return false;
    char line[64];
    bool ok = false;
    while (fgets(line, sizeof(line), f)) {
        if (sscanf(line, "level=%d", &d.level) == 1) ok = true;
        sscanf(line, "loop=%d", &d.loop);
        sscanf(line, "score=%ld", &d.score);
        sscanf(line, "hp=%f", &d.hp);
        sscanf(line, "weapon=%d", &d.weapon);
    }
    fclose(f);
    if (d.level < 1 || d.level > 5) ok = false;
    return ok;
}

void Clear() {
    std::string p = Path();
    if (!p.empty()) remove(p.c_str());
}

} // namespace save
