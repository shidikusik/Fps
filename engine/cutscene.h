#pragma once

#include <string>
#include <vector>

// Letterboxed typewriter cutscenes over a slow cinematic camera pan.
// Skippable with click / ENTER (handled by the caller via Skip()).
class Cutscene {
public:
    using LineCallback = void (*)(int line);
    // onLine (optional) fires as each line starts — e.g. a voice-over.
    void Start(std::vector<std::string> lines, LineCallback onLine = nullptr);
    void Update(float dt);
    void Skip();
    bool Finished() const { return finished_; }
    float Progress() const;   // 0..1 across the whole scene, drives the camera
    void Draw() const;

private:
    std::vector<std::string> lines_;
    size_t line_ = 0;
    size_t voicedLine_ = (size_t)-1;
    float chars_ = 0;         // typewriter position in the current line
    float holdT_ = 0;         // pause after a line completes
    float totalT_ = 0, expectedT_ = 1;
    bool finished_ = true;
    LineCallback onLine_ = nullptr;
};
