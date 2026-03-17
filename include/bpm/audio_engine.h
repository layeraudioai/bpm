#pragma once

#include "miniaudio.h"
#include "bpm/sequencer.h"
#include <memory>

namespace bpm {

class AudioEngine {
public:
    AudioEngine(std::shared_ptr<Sequencer> sequencer);
    ~AudioEngine();

    bool start();
    void stop();

    bool isRunning() const { return running; }

private:
    ma_device device;
    std::shared_ptr<Sequencer> sequencer;
    bool running = false;

    static void audioCallback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount);
};

} // namespace bpm
