#pragma once

#include <vector>
#include <cmath>
#include <string>

namespace bpm {

enum class DrumChannel {
    KickLeft = 0,
    KickRight,
    ClosedHat,
    OpenHat,
    OpeningHat,
    Crash,
    Ride,
    SmallTom,
    MidTom,
    HighTom,
    SnareClosed,
    SnareOpen,
    SnareRim,
    Count
};

inline std::string channelToString(DrumChannel channel) {
    switch (channel) {
        case DrumChannel::KickLeft: return "Kick Left";
        case DrumChannel::KickRight: return "Kick Right";
        case DrumChannel::ClosedHat: return "Closed Hat";
        case DrumChannel::OpenHat: return "Open Hat";
        case DrumChannel::OpeningHat: return "Opening Hat";
        case DrumChannel::Crash: return "Crash";
        case DrumChannel::Ride: return "Ride";
        case DrumChannel::SmallTom: return "Small Tom";
        case DrumChannel::MidTom: return "Mid Tom";
        case DrumChannel::HighTom: return "High Tom";
        case DrumChannel::SnareClosed: return "Snare Closed";
        case DrumChannel::SnareOpen: return "Snare Open";
        case DrumChannel::SnareRim: return "Snare Rim";
        default: return "Unknown";
    }
}

class DrumSynth {
public:
    virtual ~DrumSynth() = default;
    virtual void trigger() = 0;
    virtual float process(float sampleRate) = 0;
};

// Simple synthetic drum voices
class SimpleKick : public DrumSynth {
public:
    void trigger() override;
    float process(float sampleRate) override;
private:
    float phase = 0.0f;
    float frequency = 150.0f;
    float decay = 1.0f;
    bool active = false;
};

class SimpleSnare : public DrumSynth {
public:
    void trigger() override;
    float process(float sampleRate) override;
private:
    float decay = 1.0f;
    bool active = false;
};

class SimpleHat : public DrumSynth {
public:
    void trigger() override;
    float process(float sampleRate) override;
private:
    float decay = 1.0f;
    bool active = false;
};

class SimpleTom : public DrumSynth {
public:
    void trigger() override;
    float process(float sampleRate) override;
private:
    float phase = 0.0f;
    float frequency = 250.0f;
    float decay = 1.0f;
    bool active = false;
};

class SimpleCymbal : public DrumSynth {
public:
    void trigger() override;
    float process(float sampleRate) override;
private:
    float decay = 1.0f;
    bool active = false;
};

} // namespace bpm
