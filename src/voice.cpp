#include "voice.h"
#include "localization.h"
#include "voice_data.h"
#include "raylib.h"

namespace voice {
namespace {
Sound clips[2][5];
bool ready = false;
} // namespace

void Init() {
    if (!IsAudioDeviceReady()) return;
    for (int lang = 0; lang < 2; lang++) {
        for (int i = 0; i < 5; i++) {
            Wave w{};
            w.frameCount = VOICE_INTRO[lang][i].len;
            w.sampleRate = 11025;
            w.sampleSize = 8;
            w.channels = 1;
            w.data = (void*)VOICE_INTRO[lang][i].data;
            clips[lang][i] = LoadSoundFromWave(w); // copies the data
        }
    }
    ready = true;
}

void Shutdown() {
    if (!ready) return;
    for (int lang = 0; lang < 2; lang++)
        for (int i = 0; i < 5; i++) UnloadSound(clips[lang][i]);
    ready = false;
}

void StopAll() {
    if (!ready) return;
    for (int lang = 0; lang < 2; lang++)
        for (int i = 0; i < 5; i++)
            if (IsSoundPlaying(clips[lang][i])) StopSound(clips[lang][i]);
}

void PlayIntroLine(int line) {
    if (!ready || line < 0 || line > 4) return;
    StopAll();
    int lang = loc::Get() == Lang::RU ? 1 : 0;
    SetSoundVolume(clips[lang][line], 0.9f);
    PlaySound(clips[lang][line]);
}

} // namespace voice
