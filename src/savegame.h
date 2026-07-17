#pragma once

// Checkpoint saves: written at the start of every level (and loop), so the
// player can quit anytime and CONTINUE from the last level entrance.
namespace save {

struct Data {
    int level = 1;
    int loop = 0;
    long score = 0;
    float hp = 100;
    int weapon = 0;
};

void Write(const Data& d);
bool Read(Data& d);   // false if no save exists
void Clear();

} // namespace save
