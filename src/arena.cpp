#include "arena.h"

namespace {
// Shared palette pieces
constexpr Color FLOOR_DARK  = { 18, 14, 16, 255 };
constexpr Color WIRE_YELLOW = { 255, 230, 0, 255 };
} // namespace

void Arena::AddBlock(Vector3 c, Vector3 s, Color color, Color wire) {
    Block b;
    b.box.min = { c.x - s.x * 0.5f, c.y - s.y * 0.5f, c.z - s.z * 0.5f };
    b.box.max = { c.x + s.x * 0.5f, c.y + s.y * 0.5f, c.z + s.z * 0.5f };
    b.color = color;
    b.wireColor = wire;
    blocks_.push_back(b);
}

void Arena::Init(int level) {
    blocks_.clear();
    pads_.clear();
    level_ = level;
    switch (level) {
        case 2:  BuildCatacombs(); break;
        case 3:  BuildAltar(); break;
        case 4:  BuildFurnace(); break;
        case 5:  BuildThrone(); break;
        default: BuildYard(); break;
    }
}

// --- Level 1: THE YARD — open square, mid platforms, the training ground ---
void Arena::BuildYard() {
    name_ = "THE YARD";
    spawn_ = { 0.0f, 2.0f, 30.0f };
    showcase_ = { 0, 6.0f, 0 };
    gridColor_ = { 60, 24, 28, 255 };

    const Color BLOCK_BLACK = { 24, 18, 20, 255 };
    const Color BLOCK_RED   = { 92, 10, 16, 255 };
    const Color WIRE_RED    = { 220, 30, 40, 255 };
    const Color WIRE_DIM    = { 70, 30, 34, 255 };

    AddBlock({ 0, -1.0f, 0 }, { 100, 2, 100 }, FLOOR_DARK, WIRE_DIM);

    const float W = 50.0f, WH = 14.0f, WT = 2.0f;
    AddBlock({ 0, WH * 0.5f, -W - WT * 0.5f }, { 2 * W + 2 * WT, WH, WT }, BLOCK_BLACK, WIRE_RED);
    AddBlock({ 0, WH * 0.5f,  W + WT * 0.5f }, { 2 * W + 2 * WT, WH, WT }, BLOCK_BLACK, WIRE_RED);
    AddBlock({ -W - WT * 0.5f, WH * 0.5f, 0 }, { WT, WH, 2 * W }, BLOCK_BLACK, WIRE_RED);
    AddBlock({  W + WT * 0.5f, WH * 0.5f, 0 }, { WT, WH, 2 * W }, BLOCK_BLACK, WIRE_RED);

    // Central stepped tower
    AddBlock({ 0, 1.0f, 0 }, { 14, 2, 14 }, BLOCK_RED, WIRE_YELLOW);
    AddBlock({ 0, 3.0f, 0 }, { 10, 2, 10 }, BLOCK_RED, WIRE_YELLOW);
    AddBlock({ 0, 5.0f, 0 }, { 6, 2, 6 },   BLOCK_RED, WIRE_YELLOW);

    // Ring of mid platforms
    AddBlock({ -22, 2.5f, -22 }, { 12, 1, 12 }, BLOCK_BLACK, WIRE_RED);
    AddBlock({  22, 2.5f, -22 }, { 12, 1, 12 }, BLOCK_BLACK, WIRE_RED);
    AddBlock({ -22, 2.5f,  22 }, { 12, 1, 12 }, BLOCK_BLACK, WIRE_RED);
    AddBlock({  22, 2.5f,  22 }, { 12, 1, 12 }, BLOCK_BLACK, WIRE_RED);

    // High platforms on the axes
    AddBlock({  0, 4.5f, -34 }, { 14, 1, 8 }, BLOCK_BLACK, WIRE_YELLOW);
    AddBlock({  0, 4.5f,  34 }, { 14, 1, 8 }, BLOCK_BLACK, WIRE_YELLOW);
    AddBlock({ -34, 4.5f,  0 }, { 8, 1, 14 }, BLOCK_BLACK, WIRE_YELLOW);
    AddBlock({  34, 4.5f,  0 }, { 8, 1, 14 }, BLOCK_BLACK, WIRE_YELLOW);

    // Floating slam-bait platforms
    AddBlock({ -14, 7.5f, -14 }, { 6, 1, 6 }, BLOCK_RED, WIRE_YELLOW);
    AddBlock({  14, 7.5f,  14 }, { 6, 1, 6 }, BLOCK_RED, WIRE_YELLOW);

    // Corner pillars
    AddBlock({ -36, 3.0f, -36 }, { 4, 6, 4 }, BLOCK_BLACK, WIRE_RED);
    AddBlock({  36, 3.0f, -36 }, { 4, 6, 4 }, BLOCK_BLACK, WIRE_RED);
    AddBlock({ -36, 3.0f,  36 }, { 4, 6, 4 }, BLOCK_BLACK, WIRE_RED);
    AddBlock({  36, 3.0f,  36 }, { 4, 6, 4 }, BLOCK_BLACK, WIRE_RED);

    pads_ = {
        { -40, 0.5f, -40 }, { 40, 0.5f, -40 }, { -40, 0.5f, 40 }, { 40, 0.5f, 40 },
        { 0, 0.5f, -42 },   { 0, 0.5f, 42 },   { -42, 0.5f, 0 },  { 42, 0.5f, 0 },
        { -22, 3.5f, -22 }, { 22, 3.5f, 22 },
    };
}

// --- Level 2: THE CATACOMBS — dense pillar grid, tight sightlines ---
void Arena::BuildCatacombs() {
    name_ = "THE CATACOMBS";
    spawn_ = { 0.0f, 2.0f, 34.0f };
    showcase_ = { 0, 1.5f, 0 };
    gridColor_ = { 54, 30, 18, 255 };

    const Color STONE      = { 34, 22, 26, 255 };
    const Color STONE_DEEP = { 52, 16, 30, 255 };
    const Color WIRE_EMBER = { 255, 120, 0, 255 };
    const Color WIRE_DIM   = { 90, 45, 20, 255 };

    AddBlock({ 0, -1.0f, 0 }, { 100, 2, 100 }, FLOOR_DARK, WIRE_DIM);

    const float W = 50.0f, WH = 16.0f, WT = 2.0f;
    AddBlock({ 0, WH * 0.5f, -W - WT * 0.5f }, { 2 * W + 2 * WT, WH, WT }, STONE, WIRE_EMBER);
    AddBlock({ 0, WH * 0.5f,  W + WT * 0.5f }, { 2 * W + 2 * WT, WH, WT }, STONE, WIRE_EMBER);
    AddBlock({ -W - WT * 0.5f, WH * 0.5f, 0 }, { WT, WH, 2 * W }, STONE, WIRE_EMBER);
    AddBlock({  W + WT * 0.5f, WH * 0.5f, 0 }, { WT, WH, 2 * W }, STONE, WIRE_EMBER);

    // Pillar grid (skip center and spawn lane)
    for (int gx = -2; gx <= 2; gx++) {
        for (int gz = -2; gz <= 2; gz++) {
            if (gx == 0 && gz >= 0) continue; // spawn lane stays open
            if (gx == 0 && gz == 0) continue;
            float px = gx * 16.0f, pz = gz * 16.0f;
            float h = 7.0f + ((gx + gz + 8) % 3) * 2.0f;
            AddBlock({ px, h * 0.5f, pz }, { 5, h, 5 }, STONE_DEEP, WIRE_EMBER);
        }
    }

    // Walkways connecting some pillar tops
    AddBlock({ -16, 6.5f, 0 },  { 5, 1, 27 }, STONE, WIRE_YELLOW);
    AddBlock({  16, 6.5f, 0 },  { 5, 1, 27 }, STONE, WIRE_YELLOW);
    AddBlock({ 0, 6.5f, -16 },  { 27, 1, 5 }, STONE, WIRE_YELLOW);

    // Central low altar slab
    AddBlock({ 0, 0.75f, 0 }, { 10, 1.5f, 10 }, STONE_DEEP, WIRE_YELLOW);

    pads_ = {
        { -42, 0.5f, -42 }, { 42, 0.5f, -42 }, { -42, 0.5f, 42 }, { 42, 0.5f, 42 },
        { 0, 0.5f, -44 },   { -44, 0.5f, 0 },  { 44, 0.5f, 0 },
        { -16, 7.5f, 0 },   { 16, 7.5f, 0 },   { 0, 7.5f, -16 },
    };
}

// --- Level 3: THE ALTAR — vertical spire, ring platforms, the Warden's seat ---
void Arena::BuildAltar() {
    name_ = "THE ALTAR";
    spawn_ = { 0.0f, 2.0f, 38.0f };
    showcase_ = { 0, 12.0f, 0 };
    gridColor_ = { 44, 40, 30, 255 };

    const Color OBSIDIAN = { 20, 18, 26, 255 };
    const Color BONE     = { 70, 62, 48, 255 };
    const Color WIRE_GOLD = { 255, 220, 120, 255 };
    const Color WIRE_RED  = { 230, 30, 40, 255 };
    const Color WIRE_DIM  = { 70, 60, 40, 255 };

    AddBlock({ 0, -1.0f, 0 }, { 110, 2, 110 }, FLOOR_DARK, WIRE_DIM);

    const float W = 55.0f, WH = 22.0f, WT = 2.0f;
    AddBlock({ 0, WH * 0.5f, -W - WT * 0.5f }, { 2 * W + 2 * WT, WH, WT }, OBSIDIAN, WIRE_RED);
    AddBlock({ 0, WH * 0.5f,  W + WT * 0.5f }, { 2 * W + 2 * WT, WH, WT }, OBSIDIAN, WIRE_RED);
    AddBlock({ -W - WT * 0.5f, WH * 0.5f, 0 }, { WT, WH, 2 * W }, OBSIDIAN, WIRE_RED);
    AddBlock({  W + WT * 0.5f, WH * 0.5f, 0 }, { WT, WH, 2 * W }, OBSIDIAN, WIRE_RED);

    // Central spire — four tiers up to y=12
    AddBlock({ 0, 1.5f, 0 },  { 20, 3, 20 }, BONE, WIRE_GOLD);
    AddBlock({ 0, 4.5f, 0 },  { 14, 3, 14 }, BONE, WIRE_GOLD);
    AddBlock({ 0, 7.5f, 0 },  { 9, 3, 9 },   BONE, WIRE_GOLD);
    AddBlock({ 0, 10.5f, 0 }, { 5, 3, 5 },   BONE, WIRE_RED);

    // Mid-height ring platforms
    AddBlock({ -28, 4.0f, 0 },  { 10, 1, 10 }, OBSIDIAN, WIRE_GOLD);
    AddBlock({  28, 4.0f, 0 },  { 10, 1, 10 }, OBSIDIAN, WIRE_GOLD);
    AddBlock({ 0, 4.0f, -28 },  { 10, 1, 10 }, OBSIDIAN, WIRE_GOLD);
    AddBlock({ 0, 4.0f,  28 },  { 10, 1, 10 }, OBSIDIAN, WIRE_GOLD);

    // High corner perches
    AddBlock({ -34, 9.0f, -34 }, { 8, 1, 8 }, OBSIDIAN, WIRE_RED);
    AddBlock({  34, 9.0f, -34 }, { 8, 1, 8 }, OBSIDIAN, WIRE_RED);
    AddBlock({ -34, 9.0f,  34 }, { 8, 1, 8 }, OBSIDIAN, WIRE_RED);
    AddBlock({  34, 9.0f,  34 }, { 8, 1, 8 }, OBSIDIAN, WIRE_RED);

    // Bridges from ring to spire tier 2
    AddBlock({ -14, 4.0f, 0 }, { 18, 1, 4 }, BONE, WIRE_DIM);
    AddBlock({  14, 4.0f, 0 }, { 18, 1, 4 }, BONE, WIRE_DIM);

    pads_ = {
        { -46, 0.5f, -46 }, { 46, 0.5f, -46 }, { -46, 0.5f, 46 }, { 46, 0.5f, 46 },
        { 0, 0.5f, -48 },   { -48, 0.5f, 0 },  { 48, 0.5f, 0 },
        { -28, 5.0f, 0 },   { 28, 5.0f, 0 },   { 0, 12.5f, 0 },
    };
}

// --- Level 4: THE FURNACE — concentric heat rings, chimney stacks ---
void Arena::BuildFurnace() {
    name_ = "THE FURNACE";
    spawn_ = { 0.0f, 2.0f, 40.0f };
    showcase_ = { 0, 2.0f, 0 };
    gridColor_ = { 70, 30, 10, 255 };
    bossPad_ = { 0, 4.0f, 0 };

    const Color IRON   = { 30, 18, 14, 255 };
    const Color EMBER  = { 120, 40, 8, 255 };
    const Color W_FIRE = { 255, 120, 0, 255 };
    const Color W_HOT  = { 255, 200, 0, 255 };
    const Color W_DIM  = { 90, 40, 16, 255 };

    AddBlock({ 0, -1.0f, 0 }, { 105, 2, 105 }, FLOOR_DARK, W_DIM);

    const float W = 52.0f, WH = 18.0f, WT = 2.0f;
    AddBlock({ 0, WH * 0.5f, -W - WT * 0.5f }, { 2 * W + 2 * WT, WH, WT }, IRON, W_FIRE);
    AddBlock({ 0, WH * 0.5f,  W + WT * 0.5f }, { 2 * W + 2 * WT, WH, WT }, IRON, W_FIRE);
    AddBlock({ -W - WT * 0.5f, WH * 0.5f, 0 }, { WT, WH, 2 * W }, IRON, W_FIRE);
    AddBlock({  W + WT * 0.5f, WH * 0.5f, 0 }, { WT, WH, 2 * W }, IRON, W_FIRE);

    // central crucible: raised ring with a hot core
    AddBlock({ 0, 1.0f, 0 }, { 12, 2, 12 }, EMBER, W_HOT);
    // broken ring walls around it (gaps on the diagonals)
    AddBlock({ 0, 2.0f, -18 }, { 24, 4, 3 }, IRON, W_FIRE);
    AddBlock({ 0, 2.0f,  18 }, { 24, 4, 3 }, IRON, W_FIRE);
    AddBlock({ -18, 2.0f, 0 }, { 3, 4, 24 }, IRON, W_FIRE);
    AddBlock({  18, 2.0f, 0 }, { 3, 4, 24 }, IRON, W_FIRE);

    // outer ring walkway with ramp-platform steps
    AddBlock({ -30, 3.0f, -30 }, { 14, 1, 6 }, IRON, W_HOT);
    AddBlock({  30, 3.0f, -30 }, { 14, 1, 6 }, IRON, W_HOT);
    AddBlock({ -30, 3.0f,  30 }, { 14, 1, 6 }, IRON, W_HOT);
    AddBlock({  30, 3.0f,  30 }, { 14, 1, 6 }, IRON, W_HOT);
    AddBlock({ 0, 5.5f, -36 }, { 10, 1, 8 }, EMBER, W_HOT);
    AddBlock({ 0, 5.5f,  36 }, { 10, 1, 8 }, EMBER, W_HOT);

    // chimney stacks
    AddBlock({ -40, 6.0f, 0 },  { 5, 12, 5 }, IRON, W_FIRE);
    AddBlock({  40, 6.0f, 0 },  { 5, 12, 5 }, IRON, W_FIRE);
    AddBlock({ -24, 4.5f, -24 },{ 4, 9, 4 },  EMBER, W_FIRE);
    AddBlock({  24, 4.5f, 24 }, { 4, 9, 4 },  EMBER, W_FIRE);

    pads_ = {
        { -44, 0.5f, -44 }, { 44, 0.5f, -44 }, { -44, 0.5f, 44 }, { 44, 0.5f, 44 },
        { 0, 0.5f, -46 },   { -46, 0.5f, 0 },  { 46, 0.5f, 0 },
        { -30, 4.0f, -30 }, { 30, 4.0f, 30 },  { 0, 6.5f, -36 },
    };
}

// --- Level 5: THE THRONE — grand staircase to the Warden's seat ---
void Arena::BuildThrone() {
    name_ = "THE THRONE";
    spawn_ = { 0.0f, 2.0f, 44.0f };
    showcase_ = { 0, 11.0f, -6.0f };
    gridColor_ = { 50, 44, 34, 255 };
    bossPad_ = { 0, 13.0f, -30 };

    const Color MARBLE = { 40, 38, 44, 255 };
    const Color GOLDBLK = { 70, 58, 30, 255 };
    const Color W_GOLD = { 255, 220, 120, 255 };
    const Color W_RED  = { 230, 30, 40, 255 };
    const Color W_DIM  = { 70, 62, 48, 255 };

    AddBlock({ 0, -1.0f, 0 }, { 110, 2, 120 }, FLOOR_DARK, W_DIM);

    const float W = 55.0f, D = 60.0f, WH = 24.0f, WT = 2.0f;
    AddBlock({ 0, WH * 0.5f, -D - WT * 0.5f }, { 2 * W + 2 * WT, WH, WT }, MARBLE, W_GOLD);
    AddBlock({ 0, WH * 0.5f,  D + WT * 0.5f }, { 2 * W + 2 * WT, WH, WT }, MARBLE, W_GOLD);
    AddBlock({ -W - WT * 0.5f, WH * 0.5f, 0 }, { WT, WH, 2 * D }, MARBLE, W_GOLD);
    AddBlock({  W + WT * 0.5f, WH * 0.5f, 0 }, { WT, WH, 2 * D }, MARBLE, W_GOLD);

    // grand staircase rising toward -Z, six steps
    for (int i = 0; i < 6; i++) {
        float h = 2.0f * (i + 1);
        AddBlock({ 0, h - 1.0f, -6.0f - i * 6.0f }, { 26.0f - i * 2, 2, 6 },
                 i % 2 ? GOLDBLK : MARBLE, W_GOLD);
    }
    // throne platform + the seat itself
    AddBlock({ 0, 12.0f, -42 }, { 24, 2, 16 }, GOLDBLK, W_RED);
    AddBlock({ 0, 15.0f, -48 }, { 8, 8, 3 }, MARBLE, W_RED);   // backrest
    AddBlock({ 0, 13.8f, -45 }, { 6, 1.6f, 4 }, GOLDBLK, W_GOLD); // seat

    // side colonnades
    for (int i = 0; i < 4; i++) {
        float z = 20.0f - i * 18.0f;
        AddBlock({ -22, 5.0f, z }, { 4, 10, 4 }, MARBLE, W_GOLD);
        AddBlock({  22, 5.0f, z }, { 4, 10, 4 }, MARBLE, W_GOLD);
    }
    // side galleries
    AddBlock({ -34, 6.0f, -10 }, { 12, 1, 40 }, MARBLE, W_DIM);
    AddBlock({  34, 6.0f, -10 }, { 12, 1, 40 }, MARBLE, W_DIM);

    pads_ = {
        { -46, 0.5f, 46 },  { 46, 0.5f, 46 },  { -46, 0.5f, -20 }, { 46, 0.5f, -20 },
        { 0, 0.5f, 50 },    { -34, 7.0f, -10 },{ 34, 7.0f, -10 },
        { -22, 11.0f, 20 }, { 22, 11.0f, 20 }, { 0, 13.0f, -42 },
    };
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
    float ext = level_ >= 3 ? 55.0f : 50.0f;
    for (int i = (int)-ext; i <= (int)ext; i += 5) {
        DrawLine3D({ (float)i, 0.02f, -ext }, { (float)i, 0.02f, ext }, gridColor_);
        DrawLine3D({ -ext, 0.02f, (float)i }, { ext, 0.02f, (float)i }, gridColor_);
    }
}
