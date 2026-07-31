# Claude Plays Fallout

## Project Overview

Fallout Community Edition is a C++17 re-implementation of the original Fallout (1997) game engine. It supports Windows, Linux, macOS, iOS, and Android. The binary is a drop-in replacement for the original `falloutw.exe` and requires the original game's data assets to run. This fork adds an AI agent to the game and provides an interface for controlling the game.

## Build Commands

**Configure (desktop):**
```bash
cmake -B build -D CMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build -j$(nproc)
```

**With sanitizers:**
```bash
cmake -B build -D CMAKE_BUILD_TYPE=Debug -D FALLOUT_ENABLE_ASAN=ON -D FALLOUT_ENABLE_UBSAN=ON
cmake --build build
```

**Linux 32-bit (cross-compile):**
```bash
cmake -B build -D CMAKE_BUILD_TYPE=RelWithDebInfo -D CMAKE_TOOLCHAIN_FILE=cmake/toolchain/Linux_i686.cmake
```

## Code Style

Format is enforced by `.clang-format` (WebKit-based style). Check formatting:
```bash
find src -type f \( -name "*.cc" -o -name "*.h" \) | xargs clang-format --dry-run --Werror
```

Apply formatting:
```bash
find src -type f \( -name "*.cc" -o -name "*.h" \) | xargs clang-format -i
```

Static analysis (matches CI):
```bash
cppcheck --std=c++17 src/
```

There are **no automated tests** — verification is manual (run the game) plus CI build checks.

## Architecture

### Layer Overview

```
agent/         ← Python AI agent layer (Claude)
game/          ← core game logic (actions, combat, AI, map, UI, inventory, scripts)
int/           ← script interpreter VM (70+ opcodes, procedures, exports)
plib/gnw/      ← window manager, graphics buffers, input, fonts, buttons
plib/db/       ← .dat archive reader + LZSS decompression
plib/color/    ← palette management
plib/assoc/    ← key-value data associations
platform/      ← SDL2 audio engine, FPS limiter, platform path compat
third_party/   ← adecode (audio), fpattern, SDL2
os/            ← platform packaging (android/, ios/, macos/, windows/)
```

### Key Entry Points

- **`src/game/main.cc`** — `gnw_main()`: top-level init and game loop. Initializes all subsystems, runs intro movies, death scenes.
- **`src/game/game.cc`** — `game_init()`: loads config, sets up subsystems, loads initial map/save.
- **`src/int/intrpret.cc`** — script VM: executes Fallout's compiled scripts with procedure calls, stack operations, control flow.

### Platform Abstraction

SDL2 handles graphics/input on all platforms. `src/platform/audio_engine.cc` abstracts audio. `src/platform_compat.cc` provides cross-platform filesystem helpers. Platform-specific paths and packaging live in `os/` and `src/platform/`.

### Claude Agent

The AI agent lives in a Python process and communicates with the game through sockets. The agent can control the game using a set of commands. 
