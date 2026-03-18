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
    float gain = 1.0f;   // 0.0 to 1.0+
    float pan = 0.5f;    // 0.0 (left) to 1.0 (right)
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
    void setParams(size_t index, const DrumSynthParams& params);
    const DrumSynthParams& getParams(size_t index) const;
    const std::vector<Instrument>& getInstruments() const;
    const std::vector<Instrument>& getAllParams() const;

    void clearInstruments();
    void removeInstrument(size_t index);

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
    virtual void process(float sampleRate, float* left, float* right) = 0;

protected:
    DrumSynthParams params;
    float env = 0.0f;
    bool active = false;
};

class SimpleKick : public DrumSynth {
public:
    SimpleKick(const DrumSynthParams& params);
    void process(float sampleRate, float* left, float* right) override;
private:
    float phase = 0.0f;
};

class SimpleSnare : public DrumSynth {
public:
    SimpleSnare(const DrumSynthParams& params);
    void process(float sampleRate, float* left, float* right) override;
};

class SimpleHat : public DrumSynth {
public:
    SimpleHat(const DrumSynthParams& params);
    void process(float sampleRate, float* left, float* right) override;
};

class SimpleTom : public DrumSynth {
public:
    SimpleTom(const DrumSynthParams& params);
    void process(float sampleRate, float* left, float* right) override;
private:
    float phase = 0.0f;
};

class SimpleCymbal : public DrumSynth {
public:
    SimpleCymbal(const DrumSynthParams& params);
    void process(float sampleRate, float* left, float* right) override;
};

class SimpleBeep : public DrumSynth {
public:
    SimpleBeep(const DrumSynthParams& params);
    void process(float sampleRate, float* left, float* right) override;
private:
    float phase = 0.0f;
};

} // namespace bpm
