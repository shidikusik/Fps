# BLOODRUSH

[![Flatpak](https://github.com/shidikusik/Fps/actions/workflows/flatpak.yml/badge.svg)](https://github.com/shidikusik/Fps/actions/workflows/flatpak.yml)
[![Builds](https://github.com/shidikusik/Fps/actions/workflows/builds.yml/badge.svg)](https://github.com/shidikusik/Fps/actions/workflows/builds.yml)

**English | [Русский](README.ru.md)**

A frantic fast-paced first-person arena shooter in the spirit of ULTRAKILL.
C++20 + raylib, retro low-poly, 720p render with pixel upscale, fully
procedural sound — not a single asset file. Bunny hop, dash, slide, ground
slam, a style meter and blood healing. **3 levels, story cutscenes,
4 weapons, THE WARDEN boss and an endless NG+ loop after victory.**

Runs on **Linux** (X11 & Wayland), **Windows** and **Android**.

![gameplay](docs/screenshots/combat.png)

| ![arena](docs/screenshots/arena.png) | ![cutscene](docs/screenshots/cutscene.png) |
|---|---|
| THE YARD, wave one | intro cutscene |

## Story

Earth is silent. Combat machine V-13 reactivates with empty fuel
reserves — and finds an alternative: blood. The tower calls downward,
layer by layer:

1. **THE YARD** — an open courtyard with a stepped tower.
2. **THE CATACOMBS** — a maze of pillars and overhead walkways.
3. **THE ALTAR** — a vertical spire crowned by **THE WARDEN**.

Each layer is 4 waves; the third ends with the boss. Kill the Warden and
the tower offers only one thing: deeper. LOOP 2. LOOP 3...

## Downloads (CI artifacts)

Every push builds ready-to-run packages — grab them from
[**Actions**](https://github.com/shidikusik/Fps/actions), latest green run
(artifacts download as zip archives; sign-in to GitHub required):

| Platform | Workflow | Artifact |
|---|---|---|
| Linux (Flatpak) | Flatpak | `bloodrush-x86_64.flatpak` |
| Windows 10/11 x64 | Builds | `bloodrush-windows-x86_64` (`bloodrush.exe`) |
| Android 7.0+ (arm64) | Builds | `bloodrush-android` (`bloodrush.apk`) |

- **Linux**: `flatpak install --user bloodrush-x86_64.flatpak`, then launch
  from the app menu.
- **Windows**: unzip and run `bloodrush.exe` — fully standalone, no
  dependencies.
- **Android**: install the APK (allow "unknown sources"; it is signed with
  a debug key). Touch controls: left half — movement stick, right half —
  look, on-screen buttons for fire/jump/dash/slide/weapons.

## Building from source (Linux)

Only **a C++ compiler (GCC/Clang), CMake ≥ 3.16 and git** are required.
raylib is optional — when missing, CMake downloads and builds raylib 5.5
inside the project.

### 1. Dependencies

**Arch / Manjaro / EndeavourOS:**
```sh
sudo pacman -S --needed base-devel cmake git raylib
```

**Ubuntu / Debian / Mint / Pop!_OS:**
```sh
sudo apt update
sudo apt install -y build-essential cmake git \
    libgl1-mesa-dev libx11-dev libxrandr-dev libxinerama-dev \
    libxcursor-dev libxi-dev libwayland-dev libxkbcommon-dev wayland-protocols
```

**Fedora / Nobara:**
```sh
sudo dnf install -y gcc-c++ cmake git raylib-devel
```

**openSUSE:**
```sh
sudo zypper install -y gcc-c++ cmake git \
    Mesa-libGL-devel libX11-devel libXrandr-devel libXinerama-devel \
    libXcursor-devel libXi-devel wayland-devel libxkbcommon-devel wayland-protocols-devel
```

**Void:** `sudo xbps-install -S gcc cmake git raylib-devel` ·
**Gentoo:** `sudo emerge --ask dev-build/cmake media-libs/raylib`

**Any other distro:** install `gcc`/`clang`, `cmake`, `git` and the
X11/OpenGL dev headers — the build handles the rest.

### 2. Build & run

```sh
git clone https://github.com/shidikusik/Fps.git
cd Fps
mkdir build && cd build
cmake ..
make -j$(nproc)
./bloodrush
```

### 3. (Optional) app menu entry

```sh
sudo make install
```

Installs the binary, `.desktop` file and icon — the game shows up in your
app menu without any Flatpak.

## Building for other platforms

- **Windows (cross-compile from Linux):**
  ```sh
  sudo apt install mingw-w64
  cmake -S . -B build-win -DCMAKE_TOOLCHAIN_FILE=cmake/mingw-w64-x86_64.cmake
  cmake --build build-win -j$(nproc)
  ```
  Produces a standalone `bloodrush.exe` (static runtime, system DLLs only).
- **Android:** with Android SDK + NDK installed,
  `bash android/build_apk.sh` builds and signs `build-android/bloodrush.apk`
  (arm64-v8a, NativeActivity, min SDK 24).
- **Flatpak:** see [packaging/flatpak/](packaging/flatpak/) — includes the
  Flathub submission guide.

### Troubleshooting

- **`cmake: command not found`** — install cmake (step 1).
- **`GL/gl.h` or `X11/Xlib.h` errors** — missing graphics dev headers,
  install the packages from step 1.
- **No sound** — the game runs fine without an audio device; sound is
  procedural and enables itself when one is available.
- **Wayland** — works out of the box. If your system raylib lacks Wayland,
  the game transparently runs via XWayland; for guaranteed native Wayland
  build with `cmake .. -DCMAKE_DISABLE_FIND_PACKAGE_raylib=ON`.

## Controls

| Key | Action |
|---|---|
| WASD + mouse | move / look |
| SPACE | jump (hold — bunny hop with no speed loss) |
| SHIFT | dash (3 recharging charges) |
| CTRL (grounded) | slide with a speed boost |
| CTRL (airborne) | ground slam; jumping right after landing is boosted |
| LMB | fire |
| RMB (hold) | revolver charged shot — pierces everything |
| 1–4 / wheel | switch weapons |
| ENTER / click | start, skip cutscenes, retry |
| ESC | pause |
| F11 | fullscreen |

On Android everything maps to the touch UI; on desktop you can preview it
with `BLOODRUSH_TOUCH=1 ./bloodrush`.

## Combat

- **Blood heals.** Health never regenerates on its own — only by dealing
  damage up close (within ~10 meters).
- **Revolver [1]** — instant hitscan; alt-fire (RMB) — charged piercing shot.
- **Shotgun [2]** — 10 physical pellets with spread.
- **Nailgun [3]** — automatic nail stream, spinning barrels.
- **Railcannon [4]** — 160 damage through everything, long cooldown
  (readiness ring around the crosshair).
- **Parry**: the Shooters' yellow orbs can be destroyed by your shots —
  PARRY grants style.
- **Ground slam** — AoE damage around the landing point (SLAMDUNK!).

## Enemies

- **Husk** — slow, melee.
- **Shooter** — hovering octahedron firing destructible projectiles.
- **Berserker** — fast, pounces at you.
- **THE WARDEN** — the boss atop the Altar: leaps with a shockwave ring of
  destructible shots, a health bar and a golden crown.

Style meter on the right: D → C → B → A → S → ULTRAVIOLENT; grows with
kills, weapon variety, AIRSHOT/DASHKILL, drains when passive. Rank
multiplies score.

## Movement tech

- **Bhop**: hold SPACE and strafe with the mouse mid-air — speed grows.
- **Dash-jump**: jumping during a dash keeps the full dash velocity.
- **Slide-jump**: jumping out of a slide keeps slide speed.
- **Slam storage**: jump within 0.25s after a slam for a boosted jump.

Speed is always visible bottom-left (UPS = units per second).

## Code map

| Module | Purpose |
|---|---|
| `src/player.*` | quake-style physics: bhop, dash, slide, slam, health |
| `src/weapons.*` | 4 weapons, hitscan/projectiles, primitive viewmodels |
| `src/enemies.*` | 4 enemy types, waves, levels, boss, NG+ |
| `src/arena.*` | 3 arenas built from AABB blocks, spawn pads |
| `src/style_meter.*` | D→ULTRAVIOLENT ranks, popups, score |
| `src/particles.*` | physical blood cubes and sparks |
| `src/cutscene.*` | letterboxed typewriter cutscenes |
| `src/touch.*` | Android touch controls (stick, look, buttons) |
| `src/sounds.*` | all sounds procedurally generated at startup |
| `src/shading.h` | directional light + depth fog (GLSL 330 / ES 100) |
| `src/config.h` | every movement/render constant in one place |

Balance tuning: damage and cooldowns at the top of `weapons.cpp`, enemy
health and speeds at the top of `enemies.cpp`, movement in `config.h`.

## License

MIT — see [LICENSE](LICENSE).
