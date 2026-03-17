#pragma once

#include <vector>
#include <bitset>
#include <memory>
#include <mutex>
#include "bpm/drums.h"

namespace bpm {

class Sequencer {
public:
    static constexpr int NumSteps = 64;
    static constexpr int NumTracks = static_cast<int>(DrumChannel::Count);

    Sequencer();

    void setStep(DrumChannel channel, int step, bool active);
    bool getStep(DrumChannel channel, int step) const;

    void setBPM(float bpm);
    float getBPM() const;

    void clear();
    void randomize();

    const std::vector<std::bitset<NumSteps>>& getFullGrid() const { return grid; }
    void setFullGrid(const std::vector<std::bitset<NumSteps>>& newGrid) { 
        std::lock_guard<std::mutex> lock(gridMutex);
        grid = newGrid; 
    }

    // To be called by audio callback
    void process(float sampleRate, int numSamples, float* outputBuffer);

    int getCurrentStep() const { return currentStep; }

private:
    std::vector<std::bitset<NumSteps>> grid;
    std::vector<std::unique_ptr<DrumSynth>> synths;
    mutable std::mutex gridMutex;

    float bpm = 120.0f;
    float samplesPerStep = 0.0f;
    float sampleCounter = 0.0f;
    int currentStep = 0;

    void updateTiming(float sampleRate);
};

} // namespace bpm
