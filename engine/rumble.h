#pragma once

// Gamepad rumble. Linux: kernel force-feedback (evdev) — covers DualShock 4,
// DualSense, Xbox pads natively. Windows: XInput (DS4 works through Steam
// Input / DS4Windows). Android: no-op for now.
namespace rumble {

void Init();
void Shutdown();
void SetEnabled(bool on);
bool Enabled();

// low/high: 0..1 motor strengths, ms: duration.
void Pulse(float low, float high, float ms);
void Update();   // call once per frame (Windows needs a stop timer)
void Rescan();   // retry device discovery (pad connected mid-game)

} // namespace rumble
