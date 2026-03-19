#pragma once

#include <cstdint>

namespace bpm {

    enum class DrumChannel : uint8_t {
        KICK_LEFT = 0,
        KICK_RIGHT,
        SNARE_CLOSED,
        SNARE_OPEN,
        SNARE_RIM,
        CLOSED_HAT,
        OPEN_HAT,
        OPENING_HAT,
        CRASH,
        RIDE,
        SMALL_TOM,
        MID_TOM,
        HIGH_TOM,
        MELODY_1,
        MELODY_2,
        MELODY_3,
        MELODY_4,
        MELODY_5,
        COUNT
    };

} // namespace bpm
