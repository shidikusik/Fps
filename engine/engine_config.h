#pragma once

// BloodEngine render configuration.
namespace cfg {

inline constexpr int   RENDER_W = 1280;  // internal render target
inline constexpr int   RENDER_H = 720;   // point-filtered upscale
inline constexpr float MAX_DT = 1.0f / 30.0f;

} // namespace cfg
