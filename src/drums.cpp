#include "bpm/drums.h"
#include <cstdlib>
#include <stdexcept>

namespace bpm {

// --- DrumSynth ---
DrumSynth::DrumSynth(const DrumSynthParameters& params) : params(params), env(0.0f), active(false) {}

void DrumSynth::trigger() {
    active = true;
    env = 1.0f;
}

// --- SimpleKick ---
SimpleKick::SimpleKick(const DrumSynthParameters& params) : DrumSynth(params), phase(0.0f) {}

void SimpleKick::process(float sampleRate, float* left, float* right) {
    if (!active) {
        *left = *right = 0.0f;
        return;
    }

    // Pitch envelope
    float pitch_env = env > 0.5f ? (2.0f * env - 1.0f) : 0.0f; // fast drop at the beginning
    float current_freq = 40.0f + (params.frequency - 40.0f) * pitch_env;

    float val = std::sin(phase);
    phase += (2.0f * M_PI * current_freq) / sampleRate;
    if (phase > 2.0f * M_PI) phase -= 2.0f * M_PI;

    float mono = val * env * params.volume;
    *left = mono * 0.5f; // Center pan
    *right = mono * 0.5f;
    
    // Amplitude envelope
    env -= (1.0f / (params.decay * sampleRate));

    if (env <= 0.0f) {
        env = 0.0f;
        active = false;
    }
}

// --- SimpleSnare ---
SimpleSnare::SimpleSnare(const DrumSynthParameters& params) : DrumSynth(params) {}

void SimpleSnare::process(float sampleRate, float* left, float* right) {
    if (!active) {
        *left = *right = 0.0f;
        return;
    }
    
    float noise = (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 2.0f - 1.0f;
    float mono = noise * env * params.volume;
    *left = mono * 0.5f;
    *right = mono * 0.5f;

    env -= (1.0f / (params.decay * sampleRate));

    if (env <= 0.0f) {
        env = 0.0f;
        active = false;
    }
}

// --- SimpleHat ---
SimpleHat::SimpleHat(const DrumSynthParameters& params) : DrumSynth(params) {}

void SimpleHat::process(float sampleRate, float* left, float* right) {
    if (!active) {
        *left = *right = 0.0f;
        return;
    }
    
    float noise = (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 2.0f - 1.0f;
    float mono = noise * env * params.volume;
    *left = mono * 0.5f;
    *right = mono * 0.5f;
    
    env -= (1.0f / (params.decay * sampleRate));

    if (env <= 0.0f) {
        env = 0.0f;
        active = false;
    }
}

// --- SimpleTom ---
SimpleTom::SimpleTom(const DrumSynthParameters& params) : DrumSynth(params), phase(0.0f) {}

void SimpleTom::process(float sampleRate, float* left, float* right) {
    if (!active) {
        *left = *right = 0.0f;
        return;
    }

    // Pitch envelope
    float pitch_env = env > 0.8f ? (5.0f * env - 4.0f) : 0.0f;
    float current_freq = 100.0f + (params.frequency - 100.0f) * pitch_env;
    
    float val = std::sin(phase);
    phase += (2.0f * M_PI * current_freq) / sampleRate;
    if (phase > 2.0f * M_PI) phase -= 2.0f * M_PI;

    float mono = val * env * params.volume;
    *left = mono * 0.5f;
    *right = mono * 0.5f;
    
    env -= (1.0f / (params.decay * sampleRate));

    if (env <= 0.0f) {
        env = 0.0f;
        active = false;
    }
}

// --- SimpleCymbal ---
SimpleCymbal::SimpleCymbal(const DrumSynthParameters& params) : DrumSynth(params) {}

void SimpleCymbal::process(float sampleRate, float* left, float* right) {
    if (!active) {
        *left = *right = 0.0f;
        return;
    }
    
    float noise1 = (static_cast<float>(rand()) / static_cast<float>(RAND_MAX));
    float noise2 = (static_cast<float>(rand()) / static_cast<float>(RAND_MAX));
    float noise = (noise1 - noise2);

    float mono = noise * env * params.volume;
    *left = mono * 0.5f;
    *right = mono * 0.5f;

    env -= (1.0f / (params.decay * sampleRate));

    if (env <= 0.0f) {
        env = 0.0f;
        active = false;
    }
}

// --- SimpleBeep ---
SimpleBeep::SimpleBeep(const DrumSynthParameters& params) : DrumSynth(params), phase(0.0f) {}

void SimpleBeep::process(float sampleRate, float* left, float* right) {
    if (!active) {
        *left = *right = 0.0f;
        return;
    }

    float val = std::sin(phase);
    phase += (2.0f * M_PI * params.frequency) / sampleRate;
    if (phase > 2.0f * M_PI) phase -= 2.0f * M_PI;

    float mono = val * env * params.volume;
    *left = mono * 0.5f;
    *right = mono * 0.5f;

    env -= (1.0f / (params.decay * sampleRate));

    if (env <= 0.0f) {
        env = 0.0f;
        active = false;
        phase = 0.0f;
    }
}

} // namespace bpm
