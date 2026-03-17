#pragma once

#include <vector>
#include <cmath>
#include <string>
#include <memory>
#include <map>

namespace bpm {

// --- Forward declarations ---
class DrumSynth;


// --- Kit and Synth Parameters ---
struct DrumSynthParams {
    std::string type;
    float frequency = 220.0f;
    float decay = 0.5f; // in seconds
};

class Kit {
public:
    struct Instrument {
        std::string name;
        DrumSynthParams params;
        std::string ToString() const { return name; }
    };

    Kit(const std::string& name = "default");

    const std::string& getName() const;
    void setName(const std::string& newName);

    void addInstrument(const std::string& name, const DrumSynthParams& params);
    const DrumSynthParams& getParams(size_t index) const;
    const std::vector<Instrument>& getInstruments() const;
    const std::vector<Instrument>& getAllParams() const;

    static std::shared_ptr<Kit> createDefaultKit();
    static std::unique_ptr<DrumSynth> createSynth(const DrumSynthParams& params);

private:
    std::string name;
    std::vector<Instrument> instruments;
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
