#pragma once

#include <vector>
#include <bitset>
#include <memory>
#include <mutex>
#include "bpm/drums.h"

namespace bpm {

class Sequencer {
public:
    Sequencer();

    void loadKit(std::shared_ptr<Kit> newKit);
    std::shared_ptr<Kit> getKit() const;

    void setStep(int channelIndex, int step, bool active);
    bool getStep(int channelIndex, int step) const;
    void setNumSteps(int numSteps);
    int getNumSteps() const;

    void setBPM(float bpm);
    float getBPM() const;

    void clear();
    void randomize();

    const std::vector<uint64_t> getFullGrid() const { return grid; }
    void setFullGrid(const std::vector<uint64_t>& newGrid) { 
        std::lock_guard<std::mutex> lock(gridMutex);
        grid = newGrid;
    }

    // To be called by audio callback
    void process(float sampleRate, int numSamples, float* outputBuffer);

    int getCurrentStep() const { return currentStep; }

private:
    int numSteps = 64;
    std::vector<uint64_t> grid;
    std::vector<std::unique_ptr<DrumSynth>> synths;
    std::shared_ptr<Kit> currentKit;
    mutable std::mutex gridMutex;

    float bpm = 120.0f;
    float samplesPerStep = 0.0f;
    float sampleCounter = 0.0f;
    int currentStep = 0;
    void updateTiming(float sampleRate);
};

} // namespace bpm
