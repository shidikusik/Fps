#include "rumble.h"

#include <cstdio>

namespace rumble {
namespace {
bool enabled = true;
}

void SetEnabled(bool on) { enabled = on; }
bool Enabled() { return enabled; }

} // namespace rumble

#if defined(__linux__) && !defined(__ANDROID__)
// ---------------- Linux: evdev force feedback ----------------
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <sys/ioctl.h>
#include <linux/input.h>

namespace rumble {
namespace {
int fd = -1;
int effectId = -1;
double lastScan = -10.0;

bool DeviceHasRumble(int f) {
    unsigned long features[4] = {};
    if (ioctl(f, EVIOCGBIT(EV_FF, sizeof(features)), features) < 0) return false;
    return features[FF_RUMBLE / (8 * sizeof(unsigned long))] &
           (1UL << (FF_RUMBLE % (8 * sizeof(unsigned long))));
}

void Scan() {
    for (int i = 0; i < 32; i++) {
        char path[32];
        snprintf(path, sizeof(path), "/dev/input/event%d", i);
        int f = open(path, O_RDWR | O_NONBLOCK);
        if (f < 0) continue;
        if (DeviceHasRumble(f)) { fd = f; return; }
        close(f);
    }
}
} // namespace

void Init() { Scan(); }

void Shutdown() {
    if (fd >= 0) close(fd);
    fd = -1;
    effectId = -1;
}

void Pulse(float low, float high, float ms) {
    if (!enabled) return;
    if (fd < 0) return; // pad may connect later: Update() rescans

    struct ff_effect e;
    memset(&e, 0, sizeof(e));
    e.type = FF_RUMBLE;
    e.id = effectId; // -1 = allocate, else update in place
    e.u.rumble.strong_magnitude = (unsigned short)(low * 65535.0f);
    e.u.rumble.weak_magnitude = (unsigned short)(high * 65535.0f);
    e.replay.length = (unsigned short)ms;
    if (ioctl(fd, EVIOCSFF, &e) < 0) { // device gone: drop and rescan later
        close(fd);
        fd = -1;
        effectId = -1;
        return;
    }
    effectId = e.id;

    struct input_event play;
    memset(&play, 0, sizeof(play));
    play.type = EV_FF;
    play.code = (unsigned short)effectId;
    play.value = 1;
    if (write(fd, &play, sizeof(play)) < 0) {
        close(fd);
        fd = -1;
        effectId = -1;
    }
}

void Update() {}

void Rescan() { if (fd < 0) Scan(); }
} // namespace rumble

#elif defined(_WIN32)
// ---------------- Windows: XInput (dynamic load) ----------------
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace rumble {
namespace {
struct XVibration { WORD left, right; };
typedef DWORD(WINAPI* SetStateFn)(DWORD, XVibration*);
SetStateFn xinputSetState = nullptr;
double stopAt = -1;

double NowMs() { return (double)GetTickCount64(); }
} // namespace

void Init() {
    HMODULE lib = LoadLibraryA("xinput1_4.dll");
    if (!lib) lib = LoadLibraryA("xinput9_1_0.dll");
    if (lib) xinputSetState = (SetStateFn)GetProcAddress(lib, "XInputSetState");
}

void Shutdown() {}

void Pulse(float low, float high, float ms) {
    if (!enabled || !xinputSetState) return;
    XVibration v{ (WORD)(low * 65535.0f), (WORD)(high * 65535.0f) };
    xinputSetState(0, &v);
    stopAt = NowMs() + ms;
}

void Update() {
    if (stopAt > 0 && NowMs() >= stopAt) {
        stopAt = -1;
        if (xinputSetState) {
            XVibration v{ 0, 0 };
            xinputSetState(0, &v);
        }
    }
}

void Rescan() {}
} // namespace rumble

#else
// ---------------- Android & others: no-op ----------------
namespace rumble {
void Init() {}
void Shutdown() {}
void Pulse(float, float, float) {}
void Update() {}
void Rescan() {}
} // namespace rumble
#endif
