#include "bpm/drums.h"
#include <cstdlib>
#include <stdexcept>

namespace bpm {

// --- Kit ---
Kit::Kit(const std::string& name) : name(name) {
    instruments.clear();
}

const std::string& Kit::getName() const {
    return name;
}

void Kit::setName(const std::string& newName) {
    name = newName;
}

void Kit::addInstrument(const std::string& name, const DrumSynthParams& params) {
    instruments.push_back({name, params});
}

void Kit::setParams(size_t index, const DrumSynthParams& params) {
    if (index >= instruments.size()) {
        throw std::out_of_range("Instrument index out of range");
    }
    instruments[index].params = params;
}

const DrumSynthParams& Kit::getParams(size_t index) const {
    if (index >= instruments.size()) {
        throw std::out_of_range("Instrument index out of range");
    }
    return instruments[index].params;
}

const std::vector<Kit::Instrument>& Kit::getInstruments() const {
    return instruments;
}

const std::vector<Kit::Instrument>& Kit::getAllParams() const {
    return instruments;
}

void Kit::clearInstruments() {
    instruments.clear();
}

void Kit::removeInstrument(size_t index) {
    if (index < instruments.size()) {
        instruments.erase(instruments.begin() + index);
    }
}

std::shared_ptr<Kit> Kit::createDefaultKit() {
    auto kit = std::make_shared<Kit>("default");

    kit->addInstrument("KickLeft",     {"simplekick", 120.0f, 0.5f, 1.0f, 0.4f});
    kit->addInstrument("KickRight",    {"simplekick", 120.0f, 0.5f, 1.0f, 0.6f});
    kit->addInstrument("SnareClosed",  {"simplesnare", 0.0f, 0.2f, 0.8f, 0.5f});
    kit->addInstrument("SnareOpen",    {"simplesnare", 0.0f, 0.4f, 0.8f, 0.5f});
    kit->addInstrument("SnareRim",     {"simplesnare", 0.0f, 0.1f, 0.6f, 0.5f});
    kit->addInstrument("ClosedHat",    {"simplehat", 0.0f, 0.1f, 0.5f, 0.3f});
    kit->addInstrument("OpenHat",      {"simplehat", 0.0f, 0.5f, 0.5f, 0.3f});
    kit->addInstrument("OpeningHat",   {"simplehat", 0.0f, 0.3f, 0.5f, 0.3f});
    kit->addInstrument("Crash",        {"simplecymbal", 0.0f, 1.5f, 0.7f, 0.2f});
    kit->addInstrument("Ride",         {"simplecymbal", 0.0f, 2.0f, 0.7f, 0.8f});
    kit->addInstrument("SmallTom",     {"simpletom", 300.0f, 0.3f, 0.8f, 0.4f});
    kit->addInstrument("MidTom",       {"simpletom", 250.0f, 0.4f, 0.8f, 0.5f});
    kit->addInstrument("HighTom",      {"simpletom", 200.0f, 0.5f, 0.8f, 0.6f});
    
    return kit;
}

std::unique_ptr<DrumSynth> Kit::createSynth(const DrumSynthParams& params) {
    if (params.type == "simplekick") {
        return std::make_unique<SimpleKick>(params);
    } else if (params.type == "simplesnare") {
        return std::make_unique<SimpleSnare>(params);
    } else if (params.type == "simplehat") {
        return std::make_unique<SimpleHat>(params);
    } else if (params.type == "simpletom") {
        return std::make_unique<SimpleTom>(params);
    } else if (params.type == "simplecymbal") {
        return std::make_unique<SimpleCymbal>(params);
    }
    return nullptr;
}


// --- DrumSynth ---
DrumSynth::DrumSynth(const DrumSynthParams& params) : params(params), env(0.0f), active(false) {}

void DrumSynth::trigger() {
    active = true;
    env = 1.0f;
}

// --- SimpleKick ---
SimpleKick::SimpleKick(const DrumSynthParams& params) : DrumSynth(params), phase(0.0f) {}

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

    float mono = val * env * params.gain;
    *left = mono * (1.0f - params.pan);
    *right = mono * params.pan;
    
    // Amplitude envelope
    env -= (1.0f / (params.decay * sampleRate));

    if (env <= 0.0f) {
        env = 0.0f;
        active = false;
    }
}

// --- SimpleSnare ---
SimpleSnare::SimpleSnare(const DrumSynthParams& params) : DrumSynth(params) {}

void SimpleSnare::process(float sampleRate, float* left, float* right) {
    if (!active) {
        *left = *right = 0.0f;
        return;
    }
    
    float noise = (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 2.0f - 1.0f;
    float mono = noise * env * params.gain;
    *left = mono * (1.0f - params.pan);
    *right = mono * params.pan;

    env -= (1.0f / (params.decay * sampleRate));

    if (env <= 0.0f) {
        env = 0.0f;
        active = false;
    }
}

// --- SimpleHat ---
SimpleHat::SimpleHat(const DrumSynthParams& params) : DrumSynth(params) {}

void SimpleHat::process(float sampleRate, float* left, float* right) {
    if (!active) {
        *left = *right = 0.0f;
        return;
    }
    
    float noise = (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 2.0f - 1.0f;
    float mono = noise * env * params.gain;
    *left = mono * (1.0f - params.pan);
    *right = mono * params.pan;
    
    env -= (1.0f / (params.decay * sampleRate));

    if (env <= 0.0f) {
        env = 0.0f;
        active = false;
    }
}

// --- SimpleTom ---
SimpleTom::SimpleTom(const DrumSynthParams& params) : DrumSynth(params), phase(0.0f) {}

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

    float mono = val * env * params.gain;
    *left = mono * (1.0f - params.pan);
    *right = mono * params.pan;
    
    env -= (1.0f / (params.decay * sampleRate));

    if (env <= 0.0f) {
        env = 0.0f;
        active = false;
    }
}

// --- SimpleCymbal ---
SimpleCymbal::SimpleCymbal(const DrumSynthParams& params) : DrumSynth(params) {}

void SimpleCymbal::process(float sampleRate, float* left, float* right) {
    if (!active) {
        *left = *right = 0.0f;
        return;
    }
    
    float noise1 = (static_cast<float>(rand()) / static_cast<float>(RAND_MAX));
    float noise2 = (static_cast<float>(rand()) / static_cast<float>(RAND_MAX));
    float noise = (noise1 - noise2);

    float mono = noise * env * params.gain;
    *left = mono * (1.0f - params.pan);
    *right = mono * params.pan;

    env -= (1.0f / (params.decay * sampleRate));

    if (env <= 0.0f) {
        env = 0.0f;
        active = false;
    }
}

} // namespace bpm
