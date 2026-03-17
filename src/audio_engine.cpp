#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include "bpm/audio_engine.h"
#include <iostream>

namespace bpm {

AudioEngine::AudioEngine(std::shared_ptr<Sequencer> sequencer)
    : sequencer(sequencer) {
    
    ma_device_config config = ma_device_config_init(ma_device_type_playback);
    config.playback.format   = ma_format_f32;
    config.playback.channels = 2;
    config.sampleRate        = 96000;
    config.dataCallback      = audioCallback;
    config.pUserData         = this;

    if (ma_device_init(NULL, &config, &device) != MA_SUCCESS) {
        std::cerr << "Failed to initialize playback device." << std::endl;
    }
}

AudioEngine::~AudioEngine() {
    if (running) {
        stop();
    }
    ma_device_uninit(&device);
}

bool AudioEngine::start() {
    if (ma_device_start(&device) != MA_SUCCESS) {
        std::cerr << "Failed to start playback device." << std::endl;
        return false;
    }
    running = true;
    return true;
}

void AudioEngine::stop() {
    if (ma_device_stop(&device) != MA_SUCCESS) {
        std::cerr << "Failed to stop playback device." << std::endl;
    }
    running = false;
}

void AudioEngine::audioCallback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
    AudioEngine* engine = static_cast<AudioEngine*>(pDevice->pUserData);
    if (engine && engine->sequencer) {
        engine->sequencer->process(pDevice->sampleRate, frameCount, static_cast<float*>(pOutput));
    }
}

} // namespace bpm
