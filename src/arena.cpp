#include "arena.h"

namespace {
// Palette
constexpr Color FLOOR_DARK  = { 18, 14, 16, 255 };
constexpr Color BLOCK_BLACK = { 24, 18, 20, 255 };
constexpr Color BLOCK_RED   = { 92, 10, 16, 255 };
constexpr Color WIRE_RED    = { 220, 30, 40, 255 };
constexpr Color WIRE_YELLOW = { 255, 230, 0, 255 };
constexpr Color WIRE_DIM    = { 70, 30, 34, 255 };
} // namespace

void Arena::AddBlock(Vector3 c, Vector3 s, Color color, Color wire) {
    Block b;
    b.box.min = { c.x - s.x * 0.5f, c.y - s.y * 0.5f, c.z - s.z * 0.5f };
    b.box.max = { c.x + s.x * 0.5f, c.y + s.y * 0.5f, c.z + s.z * 0.5f };
    b.color = color;
    b.wireColor = wire;
    blocks_.push_back(b);
}

void Arena::Init() {
    blocks_.clear();
    spawn_ = { 0.0f, 2.0f, 30.0f };

    // Main floor (top surface at y = 0)
    AddBlock({ 0, -1.0f, 0 }, { 100, 2, 100 }, FLOOR_DARK, WIRE_DIM);

    // Perimeter walls
    const float W = 50.0f, WH = 14.0f, WT = 2.0f;
    AddBlock({ 0, WH * 0.5f, -W - WT * 0.5f }, { 2 * W + 2 * WT, WH, WT }, BLOCK_BLACK, WIRE_RED);
    AddBlock({ 0, WH * 0.5f,  W + WT * 0.5f }, { 2 * W + 2 * WT, WH, WT }, BLOCK_BLACK, WIRE_RED);
    AddBlock({ -W - WT * 0.5f, WH * 0.5f, 0 }, { WT, WH, 2 * W }, BLOCK_BLACK, WIRE_RED);
    AddBlock({  W + WT * 0.5f, WH * 0.5f, 0 }, { WT, WH, 2 * W }, BLOCK_BLACK, WIRE_RED);

    // Central tower — the landmark. Stepped so it can be climbed by jumps.
    AddBlock({ 0, 1.0f, 0 },  { 14, 2, 14 }, BLOCK_RED, WIRE_YELLOW);
    AddBlock({ 0, 3.0f, 0 },  { 10, 2, 10 }, BLOCK_RED, WIRE_YELLOW);
    AddBlock({ 0, 5.0f, 0 },  { 6, 2, 6 },   BLOCK_RED, WIRE_YELLOW);

    // Ring of mid platforms around the tower
    AddBlock({ -22,  2.5f, -22 }, { 12, 1, 12 }, BLOCK_BLACK, WIRE_RED);
    AddBlock({  22,  2.5f, -22 }, { 12, 1, 12 }, BLOCK_BLACK, WIRE_RED);
    AddBlock({ -22,  2.5f,  22 }, { 12, 1, 12 }, BLOCK_BLACK, WIRE_RED);
    AddBlock({  22,  2.5f,  22 }, { 12, 1, 12 }, BLOCK_BLACK, WIRE_RED);

    // Higher platforms on the axes — dash/bhop routes
    AddBlock({  0, 4.5f, -34 }, { 14, 1, 8 },  BLOCK_BLACK, WIRE_YELLOW);
    AddBlock({  0, 4.5f,  34 }, { 14, 1, 8 },  BLOCK_BLACK, WIRE_YELLOW);
    AddBlock({ -34, 4.5f,  0 }, { 8, 1, 14 },  BLOCK_BLACK, WIRE_YELLOW);
    AddBlock({  34, 4.5f,  0 }, { 8, 1, 14 },  BLOCK_BLACK, WIRE_YELLOW);

    // Floating high platforms — slam bait
    AddBlock({ -14, 7.5f, -14 }, { 6, 1, 6 }, BLOCK_RED, WIRE_YELLOW);
    AddBlock({  14, 7.5f,  14 }, { 6, 1, 6 }, BLOCK_RED, WIRE_YELLOW);

    // Pillars for cover / wall-adjacent hops
    AddBlock({ -36, 3.0f, -36 }, { 4, 6, 4 }, BLOCK_BLACK, WIRE_RED);
    AddBlock({  36, 3.0f, -36 }, { 4, 6, 4 }, BLOCK_BLACK, WIRE_RED);
    AddBlock({ -36, 3.0f,  36 }, { 4, 6, 4 }, BLOCK_BLACK, WIRE_RED);
    AddBlock({  36, 3.0f,  36 }, { 4, 6, 4 }, BLOCK_BLACK, WIRE_RED);
}

void Arena::Draw() const {
    for (const Block& b : blocks_) {
        Vector3 c = {
            (b.box.min.x + b.box.max.x) * 0.5f,
            (b.box.min.y + b.box.max.y) * 0.5f,
            (b.box.min.z + b.box.max.z) * 0.5f,
        };
        Vector3 s = {
            b.box.max.x - b.box.min.x,
            b.box.max.y - b.box.min.y,
            b.box.max.z - b.box.min.z,
        };
        DrawCube(c, s.x, s.y, s.z, b.color);
        DrawCubeWires(c, s.x, s.y, s.z, b.wireColor);
    }

    // Floor grid for speed perception
    const Color grid = { 60, 24, 28, 255 };
    for (int i = -50; i <= 50; i += 5) {
        DrawLine3D({ (float)i, 0.02f, -50 }, { (float)i, 0.02f, 50 }, grid);
        DrawLine3D({ -50, 0.02f, (float)i }, { 50, 0.02f, (float)i }, grid);
    }
}
