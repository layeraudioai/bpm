#include "bpm/sequencer.h"
#include <iostream>

namespace bpm {

Sequencer::Sequencer() : grid(NumTracks) {
    // Initialize synths for each channel
    for (int i = 0; i < NumTracks; ++i) {
        DrumChannel channel = static_cast<DrumChannel>(i);
        switch (channel) {
            case DrumChannel::KickLeft:
            case DrumChannel::KickRight:
                synths.push_back(std::make_unique<SimpleKick>());
                break;
            case DrumChannel::ClosedHat:
            case DrumChannel::OpenHat:
            case DrumChannel::OpeningHat:
                synths.push_back(std::make_unique<SimpleHat>());
                break;
            case DrumChannel::Crash:
            case DrumChannel::Ride:
                synths.push_back(std::make_unique<SimpleCymbal>());
                break;
            case DrumChannel::SmallTom:
            case DrumChannel::MidTom:
            case DrumChannel::HighTom:
                synths.push_back(std::make_unique<SimpleTom>());
                break;
            case DrumChannel::SnareClosed:
            case DrumChannel::SnareOpen:
            case DrumChannel::SnareRim:
                synths.push_back(std::make_unique<SimpleSnare>());
                break;
            default:
                synths.push_back(std::make_unique<SimpleKick>());
                break;
        }
    }
}

void Sequencer::setStep(DrumChannel channel, int step, bool active) {
    std::lock_guard<std::mutex> lock(gridMutex);
    if (step >= 0 && step < NumSteps) {
        grid[static_cast<int>(channel)][step] = active;
    }
}

bool Sequencer::getStep(DrumChannel channel, int step) const {
    std::lock_guard<std::mutex> lock(gridMutex);
    if (step >= 0 && step < NumSteps) {
        return grid[static_cast<int>(channel)][step];
    }
    return false;
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
    for (int t = 0; t < NumTracks; ++t) {
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
            for (int t = 0; t < NumTracks; ++t) {
                if (grid[t][currentStep]) {
                    synths[t]->trigger();
                }
            }
        }

        // Process synths and mix
        for (int t = 0; t < NumTracks; ++t) {
            mixedSample += synths[t]->process(sampleRate);
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
