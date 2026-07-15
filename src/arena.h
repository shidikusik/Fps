#pragma once

#include "raylib.h"
#include <string>
#include <vector>

// A solid axis-aligned block of the arena.
struct Block {
    BoundingBox box;
    Color color;
    Color wireColor;
};

// The level: a set of AABB blocks the player collides with.
// Three layouts: 1 THE YARD, 2 THE CATACOMBS, 3 THE ALTAR.
class Arena {
public:
    static constexpr int NUM_LEVELS = 3;

    void Init(int level);
    void Draw() const;

    const std::vector<Block>& Blocks() const { return blocks_; }
    const std::vector<Vector3>& SpawnPads() const { return pads_; }
    Vector3 SpawnPoint() const { return spawn_; }
    const char* Name() const { return name_.c_str(); }
    int Level() const { return level_; }

private:
    void AddBlock(Vector3 center, Vector3 size, Color color, Color wire);
    void BuildYard();
    void BuildCatacombs();
    void BuildAltar();

    std::vector<Block> blocks_;
    std::vector<Vector3> pads_;
    Vector3 spawn_{};
    std::string name_;
    Color gridColor_{};
    int level_ = 1;
};
