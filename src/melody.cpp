#include "bpm/melody.h"
#include <cstdlib>
#include <iostream>

namespace bpm {

Melody::Melody() : decay(0.2f) {
    // Default to 5 melody channels as defined in sequencer_defs.h
    frequencies.resize(5);
    randomize();
}

void Melody::randomize() {
    // C Major Pentatonic Scale
    static const float scale[] = {
        261.63f, 293.66f, 329.63f, 392.00f, 440.00f, // 4
        523.25f, 587.33f, 659.25f, 783.99f, 880.00f  // 5
    };

    for (size_t i = 0; i < frequencies.size(); ++i) {
        frequencies[i] = scale[rand() % 10];
    }
}

void Melody::applyToKit(Kit& kit) const {
    // Apply the stored frequencies to the MELODY channels in the Kit
    for (size_t i = 0; i < frequencies.size(); ++i) {
        DrumChannel ch = (DrumChannel)((int)DrumChannel::MELODY_1 + i);
        if (ch < DrumChannel::COUNT) {
            kit[ch].frequency = frequencies[i];
            kit[ch].decay = decay;
        }
    }
}

bool Melody::save(const std::string& filename) const {
    std::ofstream out(filename, std::ios::binary);
    if (!out) return false;

    // Simple binary format: version (int), decay (float), count (size_t), frequencies (float[])
    int version = 1;
    out.write(reinterpret_cast<const char*>(&version), sizeof(version));
    out.write(reinterpret_cast<const char*>(&decay), sizeof(decay));
    
    size_t count = frequencies.size();
    out.write(reinterpret_cast<const char*>(&count), sizeof(count));
    
    if (count > 0) {
        out.write(reinterpret_cast<const char*>(frequencies.data()), count * sizeof(float));
    }
    
    return out.good();
}

bool Melody::load(const std::string& filename) {
    std::ifstream in(filename, std::ios::binary);
    if (!in) return false;

    int version;
    in.read(reinterpret_cast<char*>(&version), sizeof(version));
    if (version != 1) return false; // Version mismatch

    in.read(reinterpret_cast<char*>(&decay), sizeof(decay));
    
    size_t count;
    in.read(reinterpret_cast<char*>(&count), sizeof(count));
    
    // Safety check limit
    if (count > 100) return false;
    
    frequencies.resize(count);
    if (count > 0) {
        in.read(reinterpret_cast<char*>(frequencies.data()), count * sizeof(float));
    }

    return in.good();
}

} // namespace bpm
