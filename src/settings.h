#pragma once

// Persistent user settings: language, look sensitivity, master volume.
// Stored as key=value lines in the user's config dir (where available).
namespace settings {

struct Values {
    int lang = 0;            // 0 = EN, 1 = RU
    float sensitivity = 1.0f; // 0.4 .. 2.0 multiplier (mouse + touch look)
    float volume = 1.0f;      // 0 .. 1 master volume
};

Values& Get();
void Load();     // call once at startup, applies volume
void Save();
void ApplyVolume();

} // namespace settings
