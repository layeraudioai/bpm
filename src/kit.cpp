#include "bpm/kit.h"
#include "bpm/drums.h"
#include "bpm/melody.h"
#include <cstdlib>

namespace bpm {

Kit::Kit() {
    // Initialize with default parameters for all channels
    for (uint8_t i = 0; i < (uint8_t)DrumChannel::COUNT; ++i) {
        _params[(DrumChannel)i] = DrumSynthParameters();
    }

    // Generate a default random melody and apply it to the kit parameters.
    // This ensures the kit starts with a valid set of melody notes.
    Melody defaultMelody;
    // defaultMelody constructor calls randomize()
    defaultMelody.applyToKit(*this);
}

DrumSynthParameters& Kit::operator[](DrumChannel channel) {
    return _params[channel];
}

const DrumSynthParameters& Kit::at(DrumChannel channel) const {
    return _params.at(channel);
}

Kit::iterator Kit::begin() {
    return _params.begin();
}

Kit::iterator Kit::end() {
    return _params.end();
}

Kit::const_iterator Kit::begin() const {
    return _params.begin();
}

Kit::const_iterator Kit::end() const {
    return _params.end();
}

std::shared_ptr<Kit> Kit::createDefaultKit() {
    return std::make_shared<Kit>();
}

std::unique_ptr<DrumSynth> Kit::createSynth(DrumChannel channel, const DrumSynthParameters& params) {
    switch (channel) {
        case DrumChannel::KICK_LEFT:
        case DrumChannel::KICK_RIGHT:
            return std::make_unique<SimpleKick>(params);
        case DrumChannel::SNARE_CLOSED:
        case DrumChannel::SNARE_OPEN:
        case DrumChannel::SNARE_RIM:
            return std::make_unique<SimpleSnare>(params);
        case DrumChannel::CLOSED_HAT:
        case DrumChannel::OPEN_HAT:
        case DrumChannel::OPENING_HAT:
            return std::make_unique<SimpleHat>(params);
        case DrumChannel::CRASH:
        case DrumChannel::RIDE:
            return std::make_unique<SimpleCymbal>(params);
        case DrumChannel::SMALL_TOM:
        case DrumChannel::MID_TOM:
        case DrumChannel::HIGH_TOM:
            return std::make_unique<SimpleTom>(params);
        default:
            return std::make_unique<SimpleBeep>(params);
    }
}

std::string Kit::channelToString(DrumChannel channel) {
    switch (channel) {
        case DrumChannel::KICK_LEFT: return "KICK_LEFT";
        case DrumChannel::KICK_RIGHT: return "KICK_RIGHT";
        case DrumChannel::SNARE_CLOSED: return "SNARE_CLOSED";
        case DrumChannel::SNARE_OPEN: return "SNARE_OPEN";
        case DrumChannel::SNARE_RIM: return "SNARE_RIM";
        case DrumChannel::CLOSED_HAT: return "CLOSED_HAT";
        case DrumChannel::OPEN_HAT: return "OPEN_HAT";
        case DrumChannel::OPENING_HAT: return "OPENING_HAT";
        case DrumChannel::CRASH: return "CRASH";
        case DrumChannel::RIDE: return "RIDE";
        case DrumChannel::SMALL_TOM: return "SMALL_TOM";
        case DrumChannel::MID_TOM: return "MID_TOM";
        case DrumChannel::HIGH_TOM: return "HIGH_TOM";
        case DrumChannel::MELODY_1: return "MELODY_1";
        case DrumChannel::MELODY_2: return "MELODY_2";
        case DrumChannel::MELODY_3: return "MELODY_3";
        case DrumChannel::MELODY_4: return "MELODY_4";
        case DrumChannel::MELODY_5: return "MELODY_5";
        default: return "UNKNOWN";
    }
}

DrumChannel Kit::stringToChannel(const std::string& s) {
    if (s == "KICK_LEFT") return DrumChannel::KICK_LEFT;
    if (s == "KICK_RIGHT") return DrumChannel::KICK_RIGHT;
    if (s == "SNARE_CLOSED") return DrumChannel::SNARE_CLOSED;
    if (s == "SNARE_OPEN") return DrumChannel::SNARE_OPEN;
    if (s == "SNARE_RIM") return DrumChannel::SNARE_RIM;
    if (s == "CLOSED_HAT") return DrumChannel::CLOSED_HAT;
    if (s == "OPEN_HAT") return DrumChannel::OPEN_HAT;
    if (s == "OPENING_HAT") return DrumChannel::OPENING_HAT;
    if (s == "CRASH") return DrumChannel::CRASH;
    if (s == "RIDE") return DrumChannel::RIDE;
    if (s == "SMALL_TOM") return DrumChannel::SMALL_TOM;
    if (s == "MID_TOM") return DrumChannel::MID_TOM;
    if (s == "HIGH_TOM") return DrumChannel::HIGH_TOM;
    if (s == "MELODY_1") return DrumChannel::MELODY_1;
    if (s == "MELODY_2") return DrumChannel::MELODY_2;
    if (s == "MELODY_3") return DrumChannel::MELODY_3;
    if (s == "MELODY_4") return DrumChannel::MELODY_4;
    if (s == "MELODY_5") return DrumChannel::MELODY_5;
    return DrumChannel::COUNT;
}

} // namespace bpm
