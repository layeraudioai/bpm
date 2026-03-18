#include "bpm/sequencer.h"
#include <iostream>
#include <stdexcept>

namespace bpm {

Sequencer::Sequencer() : numSteps(64), currentPatternIndex(0), arrangementIndex(0), bpm(120.0f), sampleCounter(0.0f), currentStep(-1) {
    addPattern();
    loadKit(Kit::createDefaultKit());
}

void Sequencer::loadKit(std::shared_ptr<Kit> newKit) {
    if (!newKit) {
        std::cerr << "Error: tried to load a null kit. Loading default kit instead." << std::endl;
        currentKit = Kit::createDefaultKit();
    } else {
        currentKit = newKit;
    }

    synths.clear();
    
    std::lock_guard<std::mutex> lock(gridMutex);
    
    // Resize all patterns to match new kit size
    size_t numInstruments = currentKit->getInstruments().size();
    for (auto& grid : patterns) {
        if (grid.size() != numInstruments) {
            grid.resize(numInstruments, 0);
        }
    }

    synths.resize(numInstruments);
    for (size_t i = 0; i < numInstruments; ++i) {
#ifndef __GBA__
        try {
            const auto& params = currentKit->getParams(i);
            synths[i] = Kit::createSynth(params);
        } catch (const std::out_of_range& e) {
            // Should not happen with index iteration
            std::cerr << "Warning: No params for index " << i << " in kit '" << currentKit->getName() << "'. Using a default kick." << std::endl;
            synths[i] = Kit::createSynth({"simplekick", 120.0f, 0.5f});
        }
#else
        const auto& params = currentKit->getParams(i);
        synths[i] = Kit::createSynth(params);
#endif
    }
}

std::shared_ptr<Kit> Sequencer::getKit() const {
    return currentKit;
}

void Sequencer::setStep(int channelIndex, int step, bool active) {
    std::lock_guard<std::mutex> lock(gridMutex);
    if (channelIndex >= 0 && static_cast<size_t>(channelIndex) < patterns[currentPatternIndex].size() && step >= 0 && step < this->numSteps) {
        if (active) {
            patterns[currentPatternIndex][channelIndex] |= (1ULL << step);
        } else {
            patterns[currentPatternIndex][channelIndex] &= ~(1ULL << step);
        }
    }
}

bool Sequencer::getStep(int channelIndex, int step) const {
    std::lock_guard<std::mutex> lock(gridMutex);
    if (channelIndex >= 0 && static_cast<size_t>(channelIndex) < patterns[currentPatternIndex].size() && step >= 0 && step < numSteps) {
        return (static_cast<uint64_t>(patterns[currentPatternIndex][channelIndex]) >> step) & 1;
    }
    return false;
}

void Sequencer::setNumSteps(int newNumSteps) {
    if (newNumSteps < 1) newNumSteps = 1;
    if (newNumSteps > 64) newNumSteps = 64;
    numSteps = newNumSteps;
}

int Sequencer::getNumSteps() const {
    return numSteps;
}


void Sequencer::setBPM(float newBpm) {
    std::lock_guard<std::mutex> lock(gridMutex);
    if (newBpm < 1.0f) newBpm = 1.0f;
    bpm = newBpm;
}

float Sequencer::getBPM() const {
    std::lock_guard<std::mutex> lock(gridMutex);
    return bpm;
}

void Sequencer::clear() {
    std::lock_guard<std::mutex> lock(gridMutex);
    for (auto& track : patterns[currentPatternIndex]) {
        track = 0;
    }
}

void Sequencer::randomize() {
    std::lock_guard<std::mutex> lock(gridMutex);
    for (size_t t = 0; t < patterns[currentPatternIndex].size(); ++t) {
        for (int s = 0; s < numSteps; ++s) {
            // ~5% chance of a trigger per step per track
            if (std::rand() % 100 < 5) {
                patterns[currentPatternIndex][t] |= (1ULL << s);
            } else {
                patterns[currentPatternIndex][t] &= ~(1ULL << s);
            }
        }
    }
}

void Sequencer::addPattern() {
    std::lock_guard<std::mutex> lock(gridMutex);
    size_t numInstruments = currentKit ? currentKit->getInstruments().size() : 0;
    patterns.push_back(std::vector<uint64_t>(numInstruments, 0));
}

void Sequencer::removePattern(int index) {
    std::lock_guard<std::mutex> lock(gridMutex);
    if (patterns.size() <= 1) return;
    if (index >= 0 && index < (int)patterns.size()) {
        patterns.erase(patterns.begin() + index);
        if (currentPatternIndex >= (int)patterns.size()) {
            currentPatternIndex = (int)patterns.size() - 1;
        }
        // Also update arrangement to remove invalid indices
        std::vector<int> newArr;
        for (int p : arrangement) {
            if (p < index) newArr.push_back(p);
            else if (p > index) newArr.push_back(p - 1);
        }
        arrangement = newArr;
    }
}

void Sequencer::switchPattern(int index) {
    std::lock_guard<std::mutex> lock(gridMutex);
    if (index >= 0 && index < (int)patterns.size()) {
        currentPatternIndex = index;
    }
}

void Sequencer::setArrangement(const std::vector<int>& newArrangement) {
    std::lock_guard<std::mutex> lock(gridMutex);
    arrangement = newArrangement;
    arrangementIndex = 0;
    if (!arrangement.empty() && songMode) {
        currentPatternIndex = arrangement[0];
    }
}

void Sequencer::updateTiming(float sampleRate) {
    // BPM is beats per minute. A step is 1/4 of a beat (assuming 16th notes).
    samplesPerStep = (60.0f / bpm / 4.0f) * sampleRate;
}

void Sequencer::process(float sampleRate, int numSamples, float* outputBuffer) {
    std::lock_guard<std::mutex> lock(gridMutex);
    updateTiming(sampleRate);

    for (int s = 0; s < numSamples; ++s) {
        // Advance sequencer clock
        sampleCounter += 1.0f;
        if (sampleCounter >= samplesPerStep) {
            sampleCounter -= samplesPerStep;
            int nextStep = (currentStep + 1) % numSteps;
            
            // In song mode, if we wrap, move to the next pattern
            if (nextStep == 0 && currentStep != -1 && songMode) {
                if (!arrangement.empty()) {
                    arrangementIndex = (arrangementIndex + 1) % arrangement.size();
                    currentPatternIndex = arrangement[arrangementIndex];
                } else {
                    currentPatternIndex = (currentPatternIndex + 1) % patterns.size();
                }
            }
            
            currentStep = nextStep;

            // Trigger synths for current step
            const auto& activeGrid = patterns[currentPatternIndex];
            for (size_t t = 0; t < activeGrid.size(); ++t) {
                if (((static_cast<uint64_t>(activeGrid[t]) >> currentStep) & 1) && t < synths.size() && synths[t]) {
                    synths[t]->trigger();
                }
            }
        }

        // Process synths and mix
        float leftSum = 0.0f;
        float rightSum = 0.0f;
        for (size_t t = 0; t < synths.size(); ++t) {
            if (synths[t]) {
                float left = 0.0f, right = 0.0f;
                synths[t]->process(sampleRate, &left, &right);
                leftSum += left;
                rightSum += right;
            }
        }

        // Clip/Limit (very basic)
        if (leftSum > 1.0f) leftSum = 1.0f;
        if (leftSum < -1.0f) leftSum = -1.0f;
        if (rightSum > 1.0f) rightSum = 1.0f;
        if (rightSum < -1.0f) rightSum = -1.0f;

        // Store in buffer (stereo interleaved)
        outputBuffer[s * 2] = leftSum;
        outputBuffer[s * 2 + 1] = rightSum;
    }
}

} // namespace bpm
