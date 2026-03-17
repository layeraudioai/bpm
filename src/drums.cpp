#include "bpm/drums.h"
#include <cstdlib>

namespace bpm {

// --- SimpleKick ---
void SimpleKick::trigger() {
    phase = 0.0f;
    frequency = 150.0f;
    decay = 1.0f;
    active = true;
}

float SimpleKick::process(float sampleRate) {
    if (!active) return 0.0f;
    
    float val = std::sin(phase);
    phase += (2.0f * M_PI * frequency) / sampleRate;
    
    // Frequency sweep down
    frequency *= (1.0f - 0.005f); 
    if (frequency < 40.0f) frequency = 40.0f;
    
    // Decay amplitude
    decay *= (1.0f - 0.002f);
    if (decay < 0.001f) {
        decay = 0.0f;
        active = false;
    }
    
    return val * decay;
}

// --- SimpleSnare ---
void SimpleSnare::trigger() {
    decay = 1.0f;
    active = true;
}

float SimpleSnare::process(float sampleRate) {
    if (!active) return 0.0f;
    
    // Filtered noise-like snare
    float noise = (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 2.0f - 1.0f;
    
    decay *= (1.0f - 0.003f);
    if (decay < 0.001f) {
        decay = 0.0f;
        active = false;
    }
    
    return noise * decay;
}

// --- SimpleHat ---
void SimpleHat::trigger() {
    decay = 1.0f;
    active = true;
}

float SimpleHat::process(float sampleRate) {
    if (!active) return 0.0f;
    
    // High-frequency noise for hats
    float noise = (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 2.0f - 1.0f;
    
    decay *= (1.0f - 0.005f); // Short decay
    if (decay < 0.001f) {
        decay = 0.0f;
        active = false;
    }
    
    return noise * decay;
}

// --- SimpleTom ---
void SimpleTom::trigger() {
    phase = 0.0f;
    frequency = 250.0f; // Varies based on tom type
    decay = 1.0f;
    active = true;
}

float SimpleTom::process(float sampleRate) {
    if (!active) return 0.0f;
    
    float val = std::sin(phase);
    phase += (2.0f * M_PI * frequency) / sampleRate;
    
    frequency *= (1.0f - 0.002f);
    
    decay *= (1.0f - 0.001f); // Longer decay than kick
    if (decay < 0.001f) {
        decay = 0.0f;
        active = false;
    }
    
    return val * decay;
}

// --- SimpleCymbal ---
void SimpleCymbal::trigger() {
    decay = 1.0f;
    active = true;
}

float SimpleCymbal::process(float sampleRate) {
    if (!active) return 0.0f;
    
    // Complex noise/metallic sound for cymbals
    float noise = (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 2.0f - 1.0f;
    
    decay *= (1.0f - 0.0005f); // Very long decay
    if (decay < 0.001f) {
        decay = 0.0f;
        active = false;
    }
    
    return noise * decay;
}

} // namespace bpm
