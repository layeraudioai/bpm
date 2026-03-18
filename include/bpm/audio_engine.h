#pragma once
#ifndef __GBA__ // This guard is fine for including miniaudio.h
#include "miniaudio.h"
#endif
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
    std::shared_ptr<Sequencer> sequencer;
    bool running = false;

#ifndef __GBA__
    ma_device device;
    static void audioCallback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount);
#else
    // GBA-specific members can go here later
#endif
};

} // namespace bpm
