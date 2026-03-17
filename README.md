# BPM Beat Procedure Machine

A 64-step, 13-channel drum sequencer built with C++ and CMake.

## Features
- **13 Channels**: Kick Left, Kick Right, Closed Hat, Open Hat, Opening Hat, Crash, Ride, Small Tom, Mid Tom, High Tom, Snare Closed, Snare Open, Snare Rim.
- **64 Steps**: Long sequence patterns (4 bars of 16th notes).
- **Synthetic Sounds**: Real-time synthesis of drum voices.
- **Project Management**: Save and load multiple projects to a library.
- **English Phrase Input**: Control the machine with natural language.
- **Low Latency**: Powered by `miniaudio`.

## Build Instructions

To build the project:

```bash
mkdir -p build
cd build
CC=clang-15 CXX=clang++-15 CFLAGS="-fuse-ld=gold" CXXFLAGS="-fuse-ld=gold" cmake ..
make -j$(nproc)
```

## Running

### Boot Options
- `./bpm -n`: Start a new empty project.
- `./bpm -l <project_name>`: Load a saved project on boot.
- `./bpm`: Interactive startup (choose new/load).

### English Phrase Commands
While running, you can type commands:
- `save project <name>`: Save current beat to the library.
- `load project <name>`: Load a beat from the library.
- `list projects`: Show all saved beats.
- `new random beat` / `randomize`: Generate a random pattern.
- `clear` / `reset`: Clear the current pattern.
- `faster` / `slower`: Adjust BPM by 10.
- `<drum> on every <interval>`: e.g., `kick on every 4`.
- `<drum> on <step1> <step2> ...`: e.g., `snare on 5 13`.
