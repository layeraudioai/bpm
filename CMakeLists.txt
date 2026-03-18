cmake_minimum_required(VERSION 3.10)
project(bpm VERSION 0.1.0 LANGUAGES C CXX)

# Optimization: Prefer Clang if available to avoid PATH conflicts with cross-compilers
if(NOT CMAKE_SYSTEM_NAME STREQUAL "GBA")
    if(NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND NOT DEFINED ENV{CC})
        find_program(CLANG_PATH clang-15)
        if(CLANG_PATH)
            set(CMAKE_C_COMPILER clang-15)
            set(CMAKE_CXX_COMPILER clang++-15)
            message(STATUS "Found Clang 15, using it to avoid system assembler conflicts.")
        endif()
    endif()
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Handle miniaudio (it's a single-header C library, but we'll include it in our project)
include_directories(include third_party)

# Add source files
set(SOURCES
    src/main.cpp
    src/sequencer.cpp
    src/audio_engine.cpp
    src/drums.cpp
    src/parser.cpp
    src/project_manager.cpp
    src/kit_manager.cpp
)

if(CMAKE_SYSTEM_NAME STREQUAL "GBA")
    # Glob synthGBA sources (C and Assembly)
    file(GLOB SYNTHGBA_SOURCES "third_party/synthGBA/*.c" "third_party/synthGBA/*.s")
    list(APPEND SOURCES
        src/gba_input.cpp
        src/gba_display.cpp
        ${SYNTHGBA_SOURCES}
    )
    add_definitions(-DUSE_GBA_SYNTH -D__GBA__)
    include_directories(third_party/synthGBA)
else()
    add_definitions(-DUSE_MINIAUDIO)
endif()

# Create executable
add_executable(bpm ${SOURCES})

# Links
if(UNIX AND NOT APPLE AND NOT CMAKE_SYSTEM_NAME STREQUAL "GBA")
    target_link_libraries(bpm PRIVATE m pthread dl)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "GBA")
    target_link_libraries(bpm PRIVATE gba)

    # Create the .gba ROM file from the .elf executable after linking
    add_custom_command(TARGET bpm POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:bpm> ${CMAKE_BINARY_DIR}/bpm.gba
        COMMENT "Generating bpm.gba ROM...")
endif()
