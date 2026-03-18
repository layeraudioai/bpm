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

    for (const auto& instrument : kit->getInstruments()) {
        const auto& params = instrument.params;

        file << "[instrument]" << std::endl;
        file << "name=" << instrument.name << std::endl;
        file << "type=" << params.type << std::endl;
        file << "frequency=" << params.frequency << std::endl;
        file << "decay=" << params.decay << std::endl;
        file << "gain=" << params.gain << std::endl;
        file << "pan=" << params.pan << std::endl;
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
    
    std::string currentInstrumentName;
    DrumSynthParams currentParams;
    bool readingInstrument = false;

    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        if (line[0] == '[' && line.back() == ']') {
            std::string section = line.substr(1, line.length() - 2);
            if (section == "instrument") {
                if (readingInstrument) {
                    kit->addInstrument(currentInstrumentName, currentParams);
                }
                readingInstrument = true;
                currentInstrumentName = "unnamed";
                currentParams = DrumSynthParams();
            }
        } else if (readingInstrument) {
            size_t eqPos = line.find('=');
            if (eqPos == std::string::npos) continue;

            std::string paramName = line.substr(0, eqPos);
            std::string value = line.substr(eqPos + 1);

            if (paramName == "name") {
                currentInstrumentName = value;
            } else if (paramName == "type") {
                currentParams.type = value;
            } else if (paramName == "frequency") {
                currentParams.frequency = std::stof(value);
            } else if (paramName == "decay") {
                currentParams.decay = std::stof(value);
            } else if (paramName == "gain") {
                currentParams.gain = std::stof(value);
            } else if (paramName == "pan") {
                currentParams.pan = std::stof(value);
            }
        }
    }
    
    if (readingInstrument) {
        kit->addInstrument(currentInstrumentName, currentParams);
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
