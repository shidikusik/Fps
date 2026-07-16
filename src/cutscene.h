#pragma once

#include <string>
#include <vector>

// Letterboxed typewriter cutscenes over a slow cinematic camera pan.
// Skippable with click / ENTER (handled by the caller via Skip()).
class Cutscene {
public:
    // voiced = true plays THE MACHINE's voice-over per line (intro only).
    void Start(std::vector<std::string> lines, bool voiced = false);
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
    bool voiced_ = false;
};
