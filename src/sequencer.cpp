#include "bpm/sequencer.h"
#include <iostream>
#include <stdexcept>

namespace bpm {

Sequencer::Sequencer() {
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
    
    // Resize grid to match new kit size
    size_t numInstruments = currentKit->getInstruments().size();
    if (grid.size() != numInstruments) {
        grid.resize(numInstruments);
    }

    synths.resize(numInstruments);
    for (size_t i = 0; i < numInstruments; ++i) {
        try {
            const auto& params = currentKit->getParams(i);
            synths[i] = Kit::createSynth(params);
        } catch (const std::out_of_range& e) {
            // Should not happen with index iteration
            std::cerr << "Warning: No params for index " << i << " in kit '" << currentKit->getName() << "'. Using a default kick." << std::endl;
            synths[i] = Kit::createSynth({"simplekick", 120.0f, 0.5f});
        }
    }
}

std::shared_ptr<Kit> Sequencer::getKit() const {
    return currentKit;
}

void Sequencer::setStep(int channelIndex, int step, bool active) {
    std::lock_guard<std::mutex> lock(gridMutex);
    if (channelIndex >= 0 && channelIndex < grid.size() && step >= 0 && step < NumSteps) {
        grid[channelIndex][step] = active;
    }
}

bool Sequencer::getStep(int channelIndex, int step) const {
    std::lock_guard<std::mutex> lock(gridMutex);
    if (channelIndex >= 0 && channelIndex < grid.size() && step >= 0 && step < NumSteps) {
        return grid[channelIndex][step];
    }
    return false;
}

void Sequencer::setNumSteps(int newNumSteps) {
    if (newNumSteps > NumSteps) {
        std::cerr << "Error: newNumSteps exceeds maximum of " << NumSteps << ". Ignoring." << std::endl;
        return;
    }
    // No need to resize bitsets, just ignore extra steps in processing
}

int Sequencer::getNumSteps() const {
    return NumSteps;
}


void Sequencer::setBPM(float newBpm) {
    std::lock_guard<std::mutex> lock(gridMutex);
    bpm = newBpm;
}

float Sequencer::getBPM() const {
    std::lock_guard<std::mutex> lock(gridMutex);
    return bpm;
}

void Sequencer::clear() {
    std::lock_guard<std::mutex> lock(gridMutex);
    for (auto& track : grid) {
        track.reset();
    }
}

void Sequencer::randomize() {
    std::lock_guard<std::mutex> lock(gridMutex);
    for (size_t t = 0; t < grid.size(); ++t) {
        for (int s = 0; s < NumSteps; ++s) {
            // ~5% chance of a trigger per step per track
            grid[t][s] = (std::rand() % 100 < 5);
        }
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
        float mixedSample = 0.0f;

        // Advance sequencer clock
        sampleCounter += 1.0f;
        if (sampleCounter >= samplesPerStep) {
            sampleCounter -= samplesPerStep;
            currentStep = (currentStep + 1) % NumSteps;

            // Trigger synths for current step
            for (size_t t = 0; t < grid.size(); ++t) {
                if (grid[t][currentStep] && synths[t]) {
                    synths[t]->trigger();
                }
            }
        }

        // Process synths and mix
        for (size_t t = 0; t < synths.size(); ++t) {
            if (synths[t]) {
                mixedSample += synths[t]->process(sampleRate);
            }
        }

        // Clip/Limit (very basic)
        if (mixedSample > 1.0f) mixedSample = 1.0f;
        if (mixedSample < -1.0f) mixedSample = -1.0f;

        // Store in buffer (stereo interleaved)
        outputBuffer[s * 2] = mixedSample;
        outputBuffer[s * 2 + 1] = mixedSample;
    }
}

} // namespace bpm
