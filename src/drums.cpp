#include "bpm/drums.h"
#include <cstdlib>
#include <stdexcept>

namespace bpm {

// --- Kit ---
Kit::Kit(const std::string& name) : name(name) {
    params.clear();
}

const std::string& Kit::getName() const {
    return name;
}

void Kit::setName(const std::string& newName) {
    name = newName;
}

void Kit::setParams(DrumChannel channel, const DrumSynthParams& channelParams) {
    params[channel] = channelParams;
}

const DrumSynthParams& Kit::getParams(DrumChannel channel) const {
    auto it = params.find(channel);
    if (it == params.end()) {
        throw std::out_of_range("Kit does not have params for this channel: " + channelToString(channel));
    }
    return it->second;
}

std::map<DrumChannel, DrumSynthParams>& Kit::getAllParams() {
    return params;
}

std::shared_ptr<Kit> Kit::createDefaultKit() {
    auto kit = std::make_shared<Kit>("default");

    kit->setParams(DrumChannel::KickLeft,     {"simplekick", 120.0f, 0.5f});
    kit->setParams(DrumChannel::KickRight,    {"simplekick", 120.0f, 0.5f});
    kit->setParams(DrumChannel::SnareClosed,  {"simplesnare", 0.0f, 0.2f});
    kit->setParams(DrumChannel::SnareOpen,    {"simplesnare", 0.0f, 0.4f});
    kit->setParams(DrumChannel::SnareRim,     {"simplesnare", 0.0f, 0.1f});
    kit->setParams(DrumChannel::ClosedHat,    {"simplehat", 0.0f, 0.1f});
    kit->setParams(DrumChannel::OpenHat,      {"simplehat", 0.0f, 0.5f});
    kit->setParams(DrumChannel::OpeningHat,   {"simplehat", 0.0f, 0.3f});
    kit->setParams(DrumChannel::Crash,        {"simplecymbal", 0.0f, 1.5f});
    kit->setParams(DrumChannel::Ride,         {"simplecymbal", 0.0f, 2.0f});
    kit->setParams(DrumChannel::SmallTom,     {"simpletom", 300.0f, 0.3f});
    kit->setParams(DrumChannel::MidTom,       {"simpletom", 250.0f, 0.4f});
    kit->setParams(DrumChannel::HighTom,      {"simpletom", 200.0f, 0.5f});
    
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

float SimpleKick::process(float sampleRate) {
    if (!active) return 0.0f;

    // Pitch envelope
    float pitch_env = env > 0.5f ? (2.0f * env - 1.0f) : 0.0f; // fast drop at the beginning
    float current_freq = 40.0f + (params.frequency - 40.0f) * pitch_env;

    float val = std::sin(phase);
    phase += (2.0f * M_PI * current_freq) / sampleRate;
    if (phase > 2.0f * M_PI) phase -= 2.0f * M_PI;

    float output = val * env;
    
    // Amplitude envelope
    env -= (1.0f / (params.decay * sampleRate));

    if (env <= 0.0f) {
        env = 0.0f;
        active = false;
    }
    
    return output;
}

// --- SimpleSnare ---
SimpleSnare::SimpleSnare(const DrumSynthParams& params) : DrumSynth(params) {}

float SimpleSnare::process(float sampleRate) {
    if (!active) return 0.0f;
    
    float noise = (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 2.0f - 1.0f;
    float output = noise * env;

    env -= (1.0f / (params.decay * sampleRate));

    if (env <= 0.0f) {
        env = 0.0f;
        active = false;
    }
    
    return output;
}

// --- SimpleHat ---
SimpleHat::SimpleHat(const DrumSynthParams& params) : DrumSynth(params) {}

float SimpleHat::process(float sampleRate) {
    if (!active) return 0.0f;
    
    float noise = (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 2.0f - 1.0f;
    float output = noise * env;
    
    env -= (1.0f / (params.decay * sampleRate));

    if (env <= 0.0f) {
        env = 0.0f;
        active = false;
    }
    
    return output;
}

// --- SimpleTom ---
SimpleTom::SimpleTom(const DrumSynthParams& params) : DrumSynth(params), phase(0.0f) {}

float SimpleTom::process(float sampleRate) {
    if (!active) return 0.0f;

    // Pitch envelope
    float pitch_env = env > 0.8f ? (5.0f * env - 4.0f) : 0.0f;
    float current_freq = 100.0f + (params.frequency - 100.0f) * pitch_env;
    
    float val = std::sin(phase);
    phase += (2.0f * M_PI * current_freq) / sampleRate;
    if (phase > 2.0f * M_PI) phase -= 2.0f * M_PI;

    float output = val * env;
    
    env -= (1.0f / (params.decay * sampleRate));

    if (env <= 0.0f) {
        env = 0.0f;
        active = false;
    }
    
    return output;
}

// --- SimpleCymbal ---
SimpleCymbal::SimpleCymbal(const DrumSynthParams& params) : DrumSynth(params) {}

float SimpleCymbal::process(float sampleRate) {
    if (!active) return 0.0f;
    
    float noise1 = (static_cast<float>(rand()) / static_cast<float>(RAND_MAX));
    float noise2 = (static_cast<float>(rand()) / static_cast<float>(RAND_MAX));
    float noise = (noise1 - noise2);

    float output = noise * env;

    env -= (1.0f / (params.decay * sampleRate));

    if (env <= 0.0f) {
        env = 0.0f;
        active = false;
    }
    
    return output;
}

} // namespace bpm
