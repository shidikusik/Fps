#pragma once

// THE MACHINE's intro voice-over (embedded espeak lines, EN + RU).
namespace voice {

void Init();               // after InitAudioDevice (sfx::Init)
void Shutdown();
void PlayIntroLine(int line);  // 0..4, uses the current language
void StopAll();

} // namespace voice
