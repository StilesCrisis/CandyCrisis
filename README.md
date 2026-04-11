# Candy Crisis

Modernized version of Candy Crisis (originally a Mac OS 8 era puzzle game). 
Supports three build targets:

| Target | Platform | Output |
|---|---|---|
| **Web** | Any (via Emscripten) | WebAssembly — runs in browser |
| **macOS** | macOS 10.15+ | Native desktop app |
| **Windows** | Windows 10+ | Native desktop app |

The web version supports tap and swipe controls in addition to keyboard control, so it can be played on mobile phones and tablets.

---

## Building for Web (Emscripten)

### Prerequisites

Install **CMake** (3.20+), **Ninja**, and **Python 3**:

| Platform | Command |
|---|---|
| macOS | `brew install cmake ninja python3` |
| Ubuntu/Debian | `apt install cmake ninja-build python3` |
| Windows | Download from [cmake.org](https://cmake.org/download/), [ninja-build.org](https://ninja-build.org/), [python.org](https://python.org) |

Install and activate the Emscripten SDK:

```sh
git clone https://github.com/emscripten-core/emsdk.git ~/emsdk
~/emsdk/emsdk install latest
~/emsdk/emsdk activate latest
source ~/emsdk/emsdk_env.sh
```

### Configure (one-time)

```sh
source ~/emsdk/emsdk_env.sh
emcmake cmake \
    -S /path/to/candy-crisis \
    -B build-web \
    -G Ninja \
    -DCMAKE_MAKE_PROGRAM=$(which ninja)
```

### Build

```sh
source ~/emsdk/emsdk_env.sh
cmake --build build-web
```

The first build downloads and compiles Emscripten ports (SDL2, SDL2_image, libjpeg, libpng, zlib). This can be slow the first time; subsequent builds should be fast.

Deployable output lands in `game/`. Serve it with any static file host, or locally:

```sh
cd game
python3 -m http.server 8000
```

### Web output files

```
index.html            ← redirect / loader page
favicon.png
CandyCrisis.html      ← Emscripten shell
CandyCrisis.js
CandyCrisis.wasm
audio.js              ← Web Audio / libopenmpt integration
manifest.json
pkg_shared.data/js    ← shared resources (UI, music, sounds)
pkg_level_N.data/js   ← per-level packs (×13)
pkg_world1.data/js    ← world 1 clear screen
pkg_world2.data/js    ← world 2 clear screen
pkg_world3.data/js    ← world 3 clear screen + credits
```

### Web dependencies

| Library | Source |
|---|---|
| SDL2, SDL2_image | Emscripten ports (auto-downloaded at build time) |
| libopenmpt | Pre-compiled to WASM; included at `deps/libopenmpt/` |

#### Rebuilding libopenmpt

The included `libopenmpt.a` was compiled against Emscripten 5.0.4.
If you update Emscripten and the build breaks with link errors, rebuild it.
Download the autotools
source tarball from [lib.openmpt.org](https://lib.openmpt.org/libopenmpt/download/),
then:

```sh
source ~/emsdk/emsdk_env.sh
tar xf libopenmpt-*.autotools.tar.gz
cd libopenmpt-*/
emconfigure ./configure \
    --disable-examples --disable-tests \
    --without-portaudio --without-portaudiocpp \
    --without-sdl2 --without-pulseaudio \
    --without-mpg123 --without-ogg --without-vorbis --without-vorbisfile \
    --without-sndfile --without-flac --without-zlib \
    --disable-dependency-tracking
emmake make -j$(nproc 2>/dev/null || sysctl -n hw.logicalcpu)
cp .libs/libopenmpt.a /path/to/candy-crisis-web/deps/libopenmpt/lib/libopenmpt.a
```

---

## Building for macOS

### Prerequisites

```sh
brew install cmake ninja sdl2 sdl2_image
```

**FMOD Engine** (proprietary, free for non-commercial use) must be downloaded
separately from [fmod.com](https://www.fmod.com/download) and placed at:

```
deps/FMOD/api/core/inc/     ← fmod.hpp, fmod_errors.h, etc.
deps/FMOD/api/core/lib/     ← libfmod.dylib
```

### Configure (one-time)

```sh
cmake \
    -S /path/to/candy-crisis-web \
    -B build-macos \
    -G Ninja \
    -DCMAKE_MAKE_PROGRAM=$(which ninja)
```

### Build

```sh
cmake --build build-macos
```

The executable and `CandyCrisisResources/` folder land in `build-macos/`.

---

## Building for Windows

### Prerequisites

- [CMake](https://cmake.org/download/) 3.20+
- [Visual Studio Build Tools 2022](https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2022) (C++ workload) — or the full Visual Studio IDE
- [Ninja](https://ninja-build.org/)

Download the following SDKs and extract them under `deps/`:

| Library | Download | Place at |
|---|---|---|
| SDL2 VC dev package | [libsdl.org](https://www.libsdl.org/) | `deps/SDL2/` |
| SDL2_image VC dev package | [libsdl.org](https://www.libsdl.org/) | `deps/SDL2_image/` |
| FMOD Core API (Windows) | [fmod.com](https://www.fmod.com/download) | `deps/FMOD/api/core/` |

Expected layout:

```
deps/
  SDL2/include/   SDL2/lib/x64/
  SDL2_image/include/   SDL2_image/lib/x64/
  FMOD/api/core/inc/   FMOD/api/core/lib/x64/
```

### Configure (one-time)

From a **Developer PowerShell for VS 2022** prompt (or any shell with the MSVC environment loaded):

```powershell
cmake -S \path\to\candy-crisis -B build-windows -G Ninja
```

If running from a plain shell (e.g. Git Bash), load the VS environment first:

```powershell
Import-Module "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\Tools\Microsoft.VisualStudio.DevShell.dll"
Enter-VsDevShell -VsInstallPath "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools" -SkipAutomaticLocation
cmake -S \path\to\candy-crisis -B build-windows -G Ninja
```

### Build

```powershell
cmake --build build-windows
```

The executable and required DLLs land in `build-windows/`.
