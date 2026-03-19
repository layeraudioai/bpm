#pragma once

#include <vector>
#define _USE_MATH_DEFINES
#include <cmath>
#include <string>
#include <memory>
#include "bpm/kit.h"

namespace bpm {

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// --- Drum Synths ---
class DrumSynth {
public:
    DrumSynth(const DrumSynthParameters& params);
    virtual ~DrumSynth() = default;
    virtual void trigger();
    virtual void process(float sampleRate, float* left, float* right) = 0;

protected:
    DrumSynthParameters params;
    float env = 0.0f;
    bool active = false;
};

class SimpleKick : public DrumSynth {
public:
    SimpleKick(const DrumSynthParameters& params);
    void process(float sampleRate, float* left, float* right) override;
private:
    float phase = 0.0f;
};

class SimpleSnare : public DrumSynth {
public:
    SimpleSnare(const DrumSynthParameters& params);
    void process(float sampleRate, float* left, float* right) override;
};

class SimpleHat : public DrumSynth {
public:
    SimpleHat(const DrumSynthParameters& params);
    void process(float sampleRate, float* left, float* right) override;
};

class SimpleTom : public DrumSynth {
public:
    SimpleTom(const DrumSynthParameters& params);
    void process(float sampleRate, float* left, float* right) override;
private:
    float phase = 0.0f;
};

class SimpleCymbal : public DrumSynth {
public:
    SimpleCymbal(const DrumSynthParameters& params);
    void process(float sampleRate, float* left, float* right) override;
};

class SimpleBeep : public DrumSynth {
public:
    SimpleBeep(const DrumSynthParameters& params);
    void process(float sampleRate, float* left, float* right) override;
private:
    float phase = 0.0f;
};

} // namespace bpm
