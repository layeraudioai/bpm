#pragma once

#include <vector>
#include <cmath>
#include <string>
#include <memory>
#include <map>

namespace bpm {

// --- Forward declarations ---
class DrumSynth;

// --- Enums and Converters ---
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

// --- Kit and Synth Parameters ---
struct DrumSynthParams {
    std::string type;
    float frequency = 220.0f;
    float decay = 0.5f; // in seconds
};

class Kit {
public:
    Kit(const std::string& name = "default");

    const std::string& getName() const;
    void setName(const std::string& newName);

    void setParams(DrumChannel channel, const DrumSynthParams& params);
    const DrumSynthParams& getParams(DrumChannel channel) const;
    std::map<DrumChannel, DrumSynthParams>& getAllParams();

    static std::shared_ptr<Kit> createDefaultKit();
    static std::unique_ptr<DrumSynth> createSynth(const DrumSynthParams& params);

private:
    std::string name;
    std::map<DrumChannel, DrumSynthParams> params;
};

// --- Drum Synths ---
class DrumSynth {
public:
    DrumSynth(const DrumSynthParams& params);
    virtual ~DrumSynth() = default;
    virtual void trigger();
    virtual float process(float sampleRate) = 0;

protected:
    DrumSynthParams params;
    float env = 0.0f;
    bool active = false;
};

class SimpleKick : public DrumSynth {
public:
    SimpleKick(const DrumSynthParams& params);
    float process(float sampleRate) override;
private:
    float phase = 0.0f;
};

class SimpleSnare : public DrumSynth {
public:
    SimpleSnare(const DrumSynthParams& params);
    float process(float sampleRate) override;
};

class SimpleHat : public DrumSynth {
public:
    SimpleHat(const DrumSynthParams& params);
    float process(float sampleRate) override;
};

class SimpleTom : public DrumSynth {
public:
    SimpleTom(const DrumSynthParams& params);
    float process(float sampleRate) override;
private:
    float phase = 0.0f;
};

class SimpleCymbal : public DrumSynth {
public:
    SimpleCymbal(const DrumSynthParams& params);
    float process(float sampleRate) override;
};

} // namespace bpm
