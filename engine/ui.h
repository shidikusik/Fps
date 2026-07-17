#pragma once

#include "raylib.h"

// UI text rendering with an embedded pixel font (Press Start 2P) that
// covers Latin + Cyrillic, so every language renders identically on
// every platform. Call Init() after InitWindow().
namespace ui {

void Init();
void Shutdown();

void Text(const char* text, int x, int y, int size, Color color);
int  Measure(const char* text, int size);
void TextCentered(const char* text, int y, int size, Color color);

} // namespace ui
