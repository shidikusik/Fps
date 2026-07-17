#pragma once

// ============================================================
//  BLOODENGINE — a tiny C++20 game engine built on raylib.
//  Retro low-poly rendering, procedural audio, bilingual UI,
//  cutscenes, particles, touch input, persistent settings.
// ============================================================

#include "raylib.h"

namespace be {

inline constexpr const char* NAME = "BLOODENGINE";
inline constexpr const char* VERSION = "1.0.0";

// Creates the window, initializes audio, the UI font and settings.
// Call before anything else; pairs with Shutdown().
void Init(int winW, int winH, const char* title);
void Shutdown();

} // namespace be
