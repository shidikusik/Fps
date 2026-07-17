#include "sounds.h"
#include "raylib.h"

#include <cmath>
#include <cstdlib>
#include <vector>

namespace sfx {
namespace {

constexpr int   SAMPLE_RATE = 22050;
constexpr int   VOICES = 4; // overlapping plays per sound
constexpr float PI2 = 6.28318530718f;

Sound sounds[COUNT][VOICES];
int   nextVoice[COUNT] = {};
bool  ready = false;

float Rnd2() { return (float)rand() / (float)RAND_MAX * 2.0f - 1.0f; }

// Convert float samples (soft-clipped) into a playable Sound.
Sound Build(const std::vector<float>& s) {
    std::vector<short> pcm(s.size());
    for (size_t i = 0; i < s.size(); i++)
        pcm[i] = (short)(tanhf(s[i]) * 30000.0f);
    Wave w{};
    w.frameCount = (unsigned int)s.size();
    w.sampleRate = SAMPLE_RATE;
    w.sampleSize = 16;
    w.channels = 1;
    w.data = pcm.data();
    return LoadSoundFromWave(w); // copies the data
}

std::vector<float> Buf(float seconds) {
    return std::vector<float>((size_t)(seconds * SAMPLE_RATE), 0.0f);
}

// --- generators (t = seconds since start) ---

Sound GenRevolver() {
    auto s = Buf(0.22f);
    for (size_t i = 0; i < s.size(); i++) {
        float t = (float)i / SAMPLE_RATE;
        float crack = Rnd2() * expf(-t * 48);
        float punch = sinf(PI2 * (160 - 220 * t) * t) * expf(-t * 22);
        s[i] = crack * 0.9f + punch * 0.8f;
    }
    return Build(s);
}

Sound GenShotgun() {
    auto s = Buf(0.45f);
    float lp = 0; // one-pole lowpass state for a deeper boom
    for (size_t i = 0; i < s.size(); i++) {
        float t = (float)i / SAMPLE_RATE;
        lp += (Rnd2() - lp) * 0.18f;
        float boom = sinf(PI2 * (85 - 60 * t) * t) * expf(-t * 9);
        s[i] = lp * expf(-t * 14) * 1.6f + boom * 0.9f;
    }
    return Build(s);
}

Sound GenCharged() {
    auto s = Buf(0.4f);
    for (size_t i = 0; i < s.size(); i++) {
        float t = (float)i / SAMPLE_RATE;
        float f = 240 + 1300 * t;
        float body = sinf(PI2 * f * t) * expf(-t * 7);
        float sizzle = Rnd2() * expf(-t * 10) * 0.5f;
        s[i] = body * 0.8f + sizzle;
    }
    return Build(s);
}

Sound GenHit() {
    auto s = Buf(0.09f);
    for (size_t i = 0; i < s.size(); i++) {
        float t = (float)i / SAMPLE_RATE;
        s[i] = Rnd2() * expf(-t * 65) * 0.7f + sinf(PI2 * 110 * t) * expf(-t * 40) * 0.8f;
    }
    return Build(s);
}

Sound GenKill() {
    auto s = Buf(0.28f);
    float lp = 0;
    for (size_t i = 0; i < s.size(); i++) {
        float t = (float)i / SAMPLE_RATE;
        lp += (Rnd2() - lp) * 0.3f;
        float squish = lp * expf(-t * 16) * 1.4f;
        float drop = sinf(PI2 * (200 - 380 * t) * t) * expf(-t * 12) * 0.6f;
        s[i] = squish + drop;
    }
    return Build(s);
}

Sound GenHurt() {
    auto s = Buf(0.2f);
    for (size_t i = 0; i < s.size(); i++) {
        float t = (float)i / SAMPLE_RATE;
        float sq = sinf(PI2 * 105 * t) > 0 ? 1.0f : -1.0f; // square = harsh
        s[i] = sq * expf(-t * 13) * 0.55f + Rnd2() * expf(-t * 25) * 0.3f;
    }
    return Build(s);
}

Sound GenHeal() {
    auto s = Buf(0.22f);
    for (size_t i = 0; i < s.size(); i++) {
        float t = (float)i / SAMPLE_RATE;
        float f = 320 + 420 * t;
        float env = sinf(3.14159f * t / 0.22f); // smooth in-out
        s[i] = sinf(PI2 * f * t) * env * 0.35f;
    }
    return Build(s);
}

Sound GenDash() {
    auto s = Buf(0.16f);
    float lp = 0;
    for (size_t i = 0; i < s.size(); i++) {
        float t = (float)i / SAMPLE_RATE;
        lp += (Rnd2() - lp) * (0.1f + t * 2.5f); // opening filter = whoosh
        float env = sinf(3.14159f * t / 0.16f);
        s[i] = lp * env * 1.1f;
    }
    return Build(s);
}

Sound GenSlam() {
    auto s = Buf(0.5f);
    for (size_t i = 0; i < s.size(); i++) {
        float t = (float)i / SAMPLE_RATE;
        float boom = sinf(PI2 * (55 - 25 * t) * t) * expf(-t * 6);
        float crack = Rnd2() * expf(-t * 30) * 0.8f;
        s[i] = boom * 1.2f + crack;
    }
    return Build(s);
}

Sound GenParry() {
    auto s = Buf(0.2f);
    for (size_t i = 0; i < s.size(); i++) {
        float t = (float)i / SAMPLE_RATE;
        float ding = sinf(PI2 * 1500 * t) + sinf(PI2 * 2250 * t) * 0.5f;
        s[i] = ding * expf(-t * 17) * 0.5f;
    }
    return Build(s);
}

Sound GenEnemyShoot() {
    auto s = Buf(0.14f);
    for (size_t i = 0; i < s.size(); i++) {
        float t = (float)i / SAMPLE_RATE;
        float f = 820 - 3800 * t;
        if (f < 220) f = 220;
        float sq = sinf(PI2 * f * t) > 0 ? 1.0f : -1.0f;
        s[i] = sq * expf(-t * 16) * 0.35f;
    }
    return Build(s);
}

Sound GenWaveHorn() {
    auto s = Buf(0.8f);
    for (size_t i = 0; i < s.size(); i++) {
        float t = (float)i / SAMPLE_RATE;
        float vib = 1.0f + 0.012f * sinf(PI2 * 5.5f * t);
        float f = 66 * vib;
        float saw = 2.0f * (t * f - floorf(t * f + 0.5f)); // saw wave
        float env = t < 0.08f ? t / 0.08f : expf(-(t - 0.08f) * 3.2f);
        s[i] = (saw * 0.5f + sinf(PI2 * f * t) * 0.5f) * env * 0.8f;
    }
    return Build(s);
}

Sound GenNail() {
    auto s = Buf(0.06f);
    for (size_t i = 0; i < s.size(); i++) {
        float t = (float)i / SAMPLE_RATE;
        s[i] = Rnd2() * expf(-t * 90) * 0.6f + sinf(PI2 * 480 * t) * expf(-t * 70) * 0.5f;
    }
    return Build(s);
}

Sound GenRail() {
    auto s = Buf(0.55f);
    for (size_t i = 0; i < s.size(); i++) {
        float t = (float)i / SAMPLE_RATE;
        float f = 1400 - 2400 * t;
        if (f < 90) f = 90;
        float zap = sinf(PI2 * f * t) * expf(-t * 6);
        float sub = sinf(PI2 * 52 * t) * expf(-t * 5);
        float crack = Rnd2() * expf(-t * 35) * 0.8f;
        s[i] = zap * 0.7f + sub * 0.8f + crack;
    }
    return Build(s);
}

Sound GenClick() {
    auto s = Buf(0.06f);
    for (size_t i = 0; i < s.size(); i++) {
        float t = (float)i / SAMPLE_RATE;
        s[i] = sinf(PI2 * 950 * t) * expf(-t * 60) * 0.5f;
    }
    return Build(s);
}

} // namespace

void Init() {
    InitAudioDevice();
    if (!IsAudioDeviceReady()) return; // headless: Play() becomes a no-op

    Sound base[COUNT];
    base[REVOLVER] = GenRevolver();
    base[SHOTGUN] = GenShotgun();
    base[CHARGED] = GenCharged();
    base[HIT] = GenHit();
    base[KILL] = GenKill();
    base[HURT] = GenHurt();
    base[HEAL] = GenHeal();
    base[DASH] = GenDash();
    base[SLAM] = GenSlam();
    base[PARRY] = GenParry();
    base[ENEMY_SHOOT] = GenEnemyShoot();
    base[WAVE] = GenWaveHorn();
    base[CLICK] = GenClick();
    base[NAIL] = GenNail();
    base[RAIL] = GenRail();

    for (int i = 0; i < COUNT; i++) {
        sounds[i][0] = base[i];
        for (int v = 1; v < VOICES; v++)
            sounds[i][v] = LoadSoundAlias(base[i]); // shared buffer, own voice
    }
    ready = true;
}

void Shutdown() {
    if (ready) {
        for (int i = 0; i < COUNT; i++) {
            for (int v = 1; v < VOICES; v++) UnloadSoundAlias(sounds[i][v]);
            UnloadSound(sounds[i][0]);
        }
    }
    CloseAudioDevice();
}

void Play(Id id, float volume, float pitch) {
    if (!ready) return;
    Sound& s = sounds[id][nextVoice[id]];
    nextVoice[id] = (nextVoice[id] + 1) % VOICES;
    SetSoundVolume(s, volume);
    SetSoundPitch(s, pitch * (1.0f + Rnd2() * 0.05f));
    PlaySound(s);
}

} // namespace sfx
