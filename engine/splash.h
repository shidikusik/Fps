#pragma once

// BloodEngine boot splash: procedural logo (gear + blood drop), engine
// name and a loading bar. Shown once at startup; skippable.
class Splash {
public:
    void Start();
    void Update(float dt);
    void Skip() { t_ = DURATION; }
    bool Finished() const { return t_ >= DURATION; }
    void Draw() const;   // render-target coordinates, 2D only

private:
    static constexpr float DURATION = 3.0f;
    float t_ = DURATION;
};
