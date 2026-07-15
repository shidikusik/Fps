#pragma once

#include "raylib.h"
#include <vector>

// A solid axis-aligned block of the arena.
struct Block {
    BoundingBox box;
    Color color;
    Color wireColor;
};

// The level: a set of AABB blocks the player collides with.
class Arena {
public:
    void Init();
    void Draw() const;

    const std::vector<Block>& Blocks() const { return blocks_; }
    Vector3 SpawnPoint() const { return spawn_; }

private:
    void AddBlock(Vector3 center, Vector3 size, Color color, Color wire);

    std::vector<Block> blocks_;
    Vector3 spawn_{};
};
