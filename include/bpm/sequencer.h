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

    // Pattern management
    void addPattern();
    void removePattern(int index);
    void switchPattern(int index);
    int getCurrentPatternIndex() const { return currentPatternIndex; }
    int getPatternCount() const { return patterns.size(); }
    
    void setSongMode(bool enabled) { songMode = enabled; }
    bool getSongMode() const { return songMode; }

    // Arrangement management
    void setArrangement(const std::vector<int>& newArrangement);
    const std::vector<int>& getArrangement() const { return arrangement; }
    void clearArrangement() { arrangement.clear(); arrangementIndex = 0; }
    int getArrangementIndex() const { return arrangementIndex; }

    void setReadOnly(bool ro) { readOnly = ro; }
    bool isReadOnly() const { return readOnly; }

    const std::vector<uint64_t>& getActiveGrid() const { return patterns[currentPatternIndex]; }
    const std::vector<std::vector<uint64_t>>& getAllPatterns() const { return patterns; }
    
    void setAllPatterns(const std::vector<std::vector<uint64_t>>& newPatterns) {
        std::lock_guard<std::mutex> lock(gridMutex);
        patterns = newPatterns;
        if (currentPatternIndex >= (int)patterns.size()) {
            currentPatternIndex = 0;
        }
        if (patterns.empty()) {
            addPattern();
        }
    }

    // To be called by audio callback
    void process(float sampleRate, int numSamples, float* outputBuffer);

    int getCurrentStep() const { return currentStep; }

private:
    int numSteps = 64;
    std::vector<std::vector<uint64_t>> patterns;
    int currentPatternIndex = 0;
    
    std::vector<int> arrangement;
    int arrangementIndex = 0;

    bool songMode = false;
    bool readOnly = false;

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
