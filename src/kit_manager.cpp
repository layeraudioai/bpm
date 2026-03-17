#include "bpm/kit_manager.h"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cctype>

namespace bpm {

KitManager::KitManager(const std::string& kitsDir) : kitsPath(kitsDir) {
    if (!std::filesystem::exists(kitsPath)) {
        std::filesystem::create_directory(kitsPath);
    }
}

bool KitManager::save(std::shared_ptr<Kit> kit) {
    if (!kit) return false;

    std::filesystem::path filePath = kitsPath / (kit->getName() + ".kit");
    std::ofstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file for writing: " << filePath << std::endl;
        return false;
    }

    file << "# Kit: " << kit->getName() << std::endl << std::endl;

    for (const auto& [channel, params] : kit->getAllParams()) {
        std::string channelName = channelToString(channel);
        std::transform(channelName.begin(), channelName.end(), channelName.begin(),
                       [](unsigned char c){ return std::tolower(c); });
        std::replace(channelName.begin(), channelName.end(), ' ', '_');

        file << "[" << channelName << "]" << std::endl;
        file << "type=" << params.type << std::endl;
        if (params.frequency > 0) {
            file << "frequency=" << params.frequency << std::endl;
        }
        file << "decay=" << params.decay << std::endl;
        file << std::endl;
    }
    
    std::cout << "Kit '" << kit->getName() << "' saved to " << filePath << std::endl;
    return true;
}

std::shared_ptr<Kit> KitManager::load(const std::string& name) {
    std::filesystem::path filePath = kitsPath / (name + ".kit");
    if (!std::filesystem::exists(filePath)) {
        std::cerr << "Error: Kit file not found: " << filePath << std::endl;
        return nullptr;
    }

    auto kit = std::make_shared<Kit>(name);
    std::ifstream file(filePath);
    std::string line;

    std::map<std::string, DrumChannel> channelMap;
    for (int i = 0; i < static_cast<int>(DrumChannel::Count); ++i) {
        DrumChannel ch = static_cast<DrumChannel>(i);
        std::string chName = channelToString(ch);
        std::transform(chName.begin(), chName.end(), chName.begin(),
                       [](unsigned char c){ return std::tolower(c); });
        std::replace(chName.begin(), chName.end(), ' ', '_');
        channelMap[chName] = ch;
    }
    
    DrumChannel currentChannel = DrumChannel::Count;

    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        if (line[0] == '[' && line.back() == ']') {
            std::string channelName = line.substr(1, line.length() - 2);
            if (channelMap.count(channelName)) {
                currentChannel = channelMap[channelName];
            } else {
                currentChannel = DrumChannel::Count;
            }
        } else if (currentChannel != DrumChannel::Count) {
            size_t eqPos = line.find('=');
            if (eqPos == std::string::npos) continue;

            std::string paramName = line.substr(0, eqPos);
            std::string value = line.substr(eqPos + 1);

            DrumSynthParams params = kit->getParams(currentChannel);
            if (paramName == "type") {
                params.type = value;
            } else if (paramName == "frequency") {
                params.frequency = std::stof(value);
            } else if (paramName == "decay") {
                params.decay = std::stof(value);
            }
            kit->setParams(currentChannel, params);
        }
    }

    std::cout << "Kit '" << name << "' loaded." << std::endl;
    return kit;
}

std::vector<std::string> KitManager::listKits() {
    std::vector<std::string> kits;
    for (const auto& entry : std::filesystem::directory_iterator(kitsPath)) {
        if (entry.is_regular_file() && entry.path().extension() == ".kit") {
            kits.push_back(entry.path().stem().string());
        }
    }
    return kits;
}

} // namespace bpm
