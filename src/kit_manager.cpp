#include "bpm/kit_manager.h"
#include "bpm/kit.h"
#include "bpm/sequencer.h"
#ifndef __GBA__
#include <fstream>
#include <iostream>
#include <filesystem>
#endif
#include <algorithm>
#include <map>

namespace bpm {

KitManager::KitManager(const std::string& kits_path) : _kits_path(kits_path) {
#ifndef __GBA__
    if (!std::filesystem::exists(_kits_path)) {
        std::filesystem::create_directory(_kits_path);
    }
#endif
}

bool KitManager::saveKit(const std::string& name, const Kit& kit) {
#ifndef __GBA__
    std::filesystem::path filePath = std::filesystem::path(_kits_path) / (name + ".kit");
    std::ofstream file(filePath);
    if (!file.is_open()) return false;

    file << "[kit]" << std::endl;
    file << "name=" << name << std::endl;

    for (auto it = kit.begin(); it != kit.end(); ++it) {
        file << "[channel]" << std::endl;
        file << "id=" << Kit::channelToString(it->first) << std::endl;
        file << "frequency=" << it->second.frequency << std::endl;
        file << "decay=" << it->second.decay << std::endl;
        file << "tone=" << it->second.tone << std::endl;
        file << "noise=" << it->second.noise << std::endl;
        file << "volume=" << it->second.volume << std::endl;
    }
    return true;
#else
    return false;
#endif
}

bool KitManager::loadKit(const std::string& name, Sequencer& sequencer) {
#ifndef __GBA__
    std::filesystem::path filePath = std::filesystem::path(_kits_path) / (name + ".kit");
    if (!std::filesystem::exists(filePath)) return false;

    auto kit = std::make_shared<Kit>();
    std::ifstream file(filePath);
    std::string line;
    DrumChannel currentChannel = DrumChannel::COUNT;

    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        if (line[0] == '[' && line.back() == ']') {
            std::string section = line.substr(1, line.length() - 2);
            if (section == "channel") currentChannel = DrumChannel::COUNT;
        } else {
            size_t eqPos = line.find('=');
            if (eqPos == std::string::npos) continue;
            std::string key = line.substr(0, eqPos);
            std::string val = line.substr(eqPos + 1);

            if (key == "id") {
                currentChannel = Kit::stringToChannel(val);
            } else if (currentChannel != DrumChannel::COUNT) {
                auto& p = (*kit)[currentChannel];
                if (key == "frequency") p.frequency = std::atof(val.c_str());
                else if (key == "decay") p.decay = std::atof(val.c_str());
                else if (key == "tone") p.tone = std::atof(val.c_str());
                else if (key == "noise") p.noise = std::atof(val.c_str());
                else if (key == "volume") p.volume = std::atof(val.c_str());
            }
        }
    }
    sequencer.loadKit(kit);
    return true;
#else
    return false;
#endif
}

std::vector<std::string> KitManager::listKits() {
    std::vector<std::string> kits;
#ifndef __GBA__
    if (std::filesystem::exists(_kits_path)) {
        for (const auto& entry : std::filesystem::directory_iterator(_kits_path)) {
            if (entry.is_regular_file() && entry.path().extension() == ".kit") {
                kits.push_back(entry.path().stem().string());
            }
        }
    }
#endif
    return kits;
}

} // namespace bpm
