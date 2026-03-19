# BPM Beat Procedure Machine

Beat Procedure Machine for multiple platforms

## Features
- **Dynamic Number of Channels**: Kick Left, Kick Right, Closed Hat, Open Hat, Opening Hat, Crash, Ride, Small Tom, Mid Tom, High Tom, Snare Closed, Snare Open, Snare Rim, plus beeps to make melodies
- **Dynamic Number of Steps**: Long sequence patterns allowed.
- **Synthetic Sounds**: Real-time synthesis of drum voices.
- **Project Management**: Save and load multiple projects to a library.
- **English Phrase Input**: Control the machine with natural language.
- **Low Latency**: Powered by `miniaudio` or `libSynthGBA` right now.

## Build Instructions

To build the project:

make -f Makefile.{platform}

## Running on PC

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

## Running on GBA

### Boot Options
- boot the rom via a flashcard or multiboot (a gamecube or wii with swiss or homebrewchannel can be used to send via link cable and https://github.com/FIX94/gba-link-cable-rom-sender)

### Using
- press A once the rom is booted and go from there

#
- copyright 2026 xbcx && google (gemini)
- miniaudio license in third_party/miniaudio.h
- https://github.com/Wollw/SynthGBA was used as basis for GBA make and libSynthGBA
- https://github.com/xvw/gba-rom-template was used
