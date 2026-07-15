#include "cutscene.h"
#include "config.h"
#include "sounds.h"
#include "raylib.h"

#include <cmath>

namespace {
constexpr float CHARS_PER_SEC = 30.0f;
constexpr float LINE_HOLD = 1.3f;
}

void Cutscene::Start(std::vector<std::string> lines) {
    lines_ = std::move(lines);
    line_ = 0;
    chars_ = 0;
    holdT_ = 0;
    totalT_ = 0;
    finished_ = lines_.empty();
    expectedT_ = 0.01f;
    for (const auto& l : lines_)
        expectedT_ += l.size() / CHARS_PER_SEC + LINE_HOLD;
}

float Cutscene::Progress() const {
    float t = totalT_ / expectedT_;
    return t > 1 ? 1 : t;
}

void Cutscene::Update(float dt) {
    if (finished_) return;
    totalT_ += dt;
    const std::string& cur = lines_[line_];
    if (chars_ < (float)cur.size()) {
        float prev = chars_;
        chars_ += dt * CHARS_PER_SEC;
        // typewriter blips
        if ((int)(chars_ / 3) != (int)(prev / 3))
            sfx::Play(sfx::CLICK, 0.18f, 1.6f + 0.15f * (line_ % 3));
    } else {
        holdT_ += dt;
        if (holdT_ >= LINE_HOLD) {
            holdT_ = 0;
            chars_ = 0;
            line_++;
            if (line_ >= lines_.size()) finished_ = true;
        }
    }
}

void Cutscene::Draw() const {
    if (finished_) return;
    const int W = cfg::RENDER_W, H = cfg::RENDER_H;

    // letterbox bars
    int bar = (int)(H * 0.14f);
    DrawRectangle(0, 0, W, bar, BLACK);
    DrawRectangle(0, H - bar, W, bar, BLACK);

    // current line, typewriter-clipped
    const std::string& cur = lines_[line_];
    int n = (int)chars_;
    if (n > (int)cur.size()) n = (int)cur.size();
    std::string shown = cur.substr(0, n);

    bool lastLine = line_ + 1 == lines_.size();
    Color col = lastLine ? Color{ 230, 30, 40, 255 } : Color{ 235, 230, 230, 255 };
    int size = 34;
    int w = MeasureText(cur.c_str(), size); // measure the full line: no wobble
    DrawText(shown.c_str(), W / 2 - w / 2, H / 2 - size / 2, size, col);

    // progress dots
    for (size_t i = 0; i < lines_.size(); i++) {
        Color dc = i <= line_ ? Color{ 230, 30, 40, 255 } : Color{ 80, 60, 62, 255 };
        DrawRectangle(W / 2 - (int)lines_.size() * 8 + (int)i * 16, H - bar / 2, 8, 8, dc);
    }

    const char* hint = "CLICK / ENTER — SKIP";
    DrawText(hint, W - MeasureText(hint, 16) - 18, bar / 2 - 8, 16, { 120, 60, 64, 255 });
}
