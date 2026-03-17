# BPM Project Session Context

## Overview
We've built a 64-step, 13-channel drum sequencer called `bpm` using C++ and CMake. 

### Final State
- **Binary**: Compiled and tested at `~/bpm/build/bpm`.
- **System**: Real-time audio synthesis (kick, snare, hats, etc.) using `miniaudio`.
- **Input**: Command-line natural language processing thread for real-time control.
- **Persistence**: Project management system saving to `~/bpm/projects/`.

## Key Commands & Features
While the app is running:
- `save project <name>`
- `load project <name>`
- `list projects`
- `kick on every 4`
- `snare on 5 13`
- `new random beat`
- `faster` / `slower`

## Build Strategy
Due to system toolchain conflicts with `powerpc-eabi` cross-assemblers, the build is automated to prefer `clang-15` with the `gold` linker:
```bash
CC=clang-15 CXX=clang++-15 CFLAGS="-fuse-ld=gold" CXXFLAGS="-fuse-ld=gold" cmake ..
make -j$(nproc)
```

## Directory Structure
- `include/bpm/`: Header files (Sequencer, AudioEngine, DrumSynth, Parser, ProjectManager).
- `src/`: Implementation files.
- `third_party/`: `miniaudio.h`.
- `projects/`: Library of saved `.bpm` files.

## Actions Taken
1. Scaffolded C++/CMake project.
2. Implemented real-time audio callback with `miniaudio`.
3. Created 13 synthetic drum voices.
4. Added multi-threaded sequencer with mutex protection.
5. Implemented English-like phrase parsing for live editing.
6. Added project save/load library and CLI boot flags.
7. Migrated all files to `~/bpm/`.
