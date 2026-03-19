#pragma once

#include <vector>
#include <string>
#include <fstream>
#include "bpm/kit.h"
#include "bpm/sequencer_defs.h"

namespace bpm {

class Melody {
public:
    Melody();
    
    // Randomizes the melody notes based on a pentatonic scale
    void randomize();
    
    // Applies this melody's parameters (frequencies) to the given kit
    void applyToKit(Kit& kit) const;
    
    // Save to a binary .melody file
    bool save(const std::string& filename) const;
    
    // Load from a binary .melody file
    bool load(const std::string& filename);

    const std::vector<float>& getFrequencies() const { return frequencies; }
    void setFrequencies(const std::vector<float>& freqs) { frequencies = freqs; }

private:
    std::vector<float> frequencies;
    float decay;
};

} // namespace bpm
