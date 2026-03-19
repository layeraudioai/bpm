#pragma once

#include "bpm/sequencer_defs.h" // Assuming this has DrumChannel
#include <map>
#include <memory>
#include <string>

namespace bpm
{
    // Forward declaration
    class DrumSynth;

    // Parameters for a single drum synth voice
    struct DrumSynthParameters
    {
        float frequency = 120.0f; // Base pitch
        float decay = 0.4f;       // Envelope decay time
        float tone = 0.5f;        // Controls filter/harmonics, 0-1
        float noise = 0.1f;       // Amount of noise mixed in, 0-1
        float volume = 1.0f;      // Volume of the piece
    };

    class Kit
    {
    public:
        Kit();

        DrumSynthParameters& operator[](DrumChannel channel);
        const DrumSynthParameters& at(DrumChannel channel) const;

        // Iterator support allows KitManager to loop over the kit easily
        using iterator = std::map<DrumChannel, DrumSynthParameters>::iterator;
        using const_iterator = std::map<DrumChannel, DrumSynthParameters>::const_iterator;

        iterator begin();
        iterator end();
        const_iterator begin() const;
        const_iterator end() const;

        static std::shared_ptr<Kit> createDefaultKit();
        static std::unique_ptr<DrumSynth> createSynth(DrumChannel channel, const DrumSynthParameters& params);

        static std::string channelToString(DrumChannel channel);
        static DrumChannel stringToChannel(const std::string& s);

    private:
        std::map<DrumChannel, DrumSynthParameters> _params;
    };

} // namespace bpm
