#include "bpm/parser.h"
#include <algorithm>
#include <sstream>
#include <vector>
#include <iostream>
#include <cstring>
#include <cstdlib>

namespace bpm {

CommandParser::CommandParser(std::shared_ptr<Sequencer> sequencer,
                           std::shared_ptr<ProjectManager> projectManager,
                           std::shared_ptr<KitManager> kitManager)
    : sequencer(sequencer), projectManager(projectManager), kitManager(kitManager) {}

std::shared_ptr<Kit> createRandomKit() {
    auto kit = std::make_shared<Kit>("random");

    auto randFloat = [](float min, float max) {
        return min + (max - min) * (static_cast<float>(rand()) / static_cast<float>(RAND_MAX));
    };

    kit->addInstrument("KickLeft",     {"simplekick", randFloat(40.0f, 150.0f), randFloat(0.3f, 0.8f), 1.0f, 0.4f});
    kit->addInstrument("KickRight",    {"simplekick", randFloat(40.0f, 150.0f), randFloat(0.3f, 0.8f), 1.0f, 0.6f});
    kit->addInstrument("SnareClosed",  {"simplesnare", 0.0f, randFloat(0.1f, 0.3f), 0.8f, 0.5f});
    kit->addInstrument("SnareOpen",    {"simplesnare", 0.0f, randFloat(0.2f, 0.4f), 0.8f, 0.5f});
    kit->addInstrument("SnareRim",     {"simplesnare", 0.0f, randFloat(0.05f, 0.2f), 0.6f, 0.5f});
    kit->addInstrument("ClosedHat",    {"simplehat", 0.0f, randFloat(0.02f, 0.1f), 0.5f, 0.3f});
    kit->addInstrument("OpenHat",      {"simplehat", 0.0f, randFloat(0.2f, 0.4f), 0.5f, 0.3f});
    kit->addInstrument("OpeningHat",   {"simplehat", 0.0f, randFloat(0.15f, 0.3f), 0.5f, 0.3f});
    kit->addInstrument("Crash",        {"simplecymbal", 0.0f, randFloat(1.0f, 2.5f), 0.7f, 0.2f});
    kit->addInstrument("Ride",         {"simplecymbal", 0.0f, randFloat(1.5f, 3.0f), 0.7f, 0.8f});
    kit->addInstrument("SmallTom",     {"simpletom", randFloat(250.0f, 400.0f), randFloat(0.2f, 0.5f), 0.8f, 0.4f});
    kit->addInstrument("MidTom",       {"simpletom", randFloat(150.0f, 300.0f), randFloat(0.3f, 0.6f), 0.8f, 0.5f});
    kit->addInstrument("HighTom",      {"simpletom", randFloat(100.0f, 250.0f), randFloat(0.4f, 0.7f), 0.8f, 0.6f});

    return kit;
}

void CommandParser::toLower(std::string& s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return std::tolower(c); });
}

int CommandParser::stringToChannelIndex(const std::string& s) {
    auto kit = sequencer->getKit();
    auto instruments = kit->getInstruments();
    for (size_t i = 0; i < instruments.size(); ++i) {
        std::string instrName = instruments[i].name;
        std::transform(instrName.begin(), instrName.end(), instrName.begin(), [](unsigned char c){ return std::tolower(c); });
        if (instrName == s) {
            return (int)i;
        }
    }

    // Fuzzy matching for partial names (e.g. "kick" -> "KickLeft")
    for (size_t i = 0; i < instruments.size(); ++i) {
        std::string instrName = instruments[i].name;
        std::transform(instrName.begin(), instrName.end(), instrName.begin(), [](unsigned char c){ return std::tolower(c); });
        if (instrName.find(s) != std::string::npos) {
            return (int)i;
        }
    }

    return -1;
}

void CommandParser::parse(const std::string& input) {
    std::string s = input;
    toLower(s);
    std::stringstream ss(s);
    std::string command;
    ss >> command;

    if (command.empty()) return;

    // Project commands
    if (command == "save" || command == "load") {
        std::string object_type;
        ss >> object_type;
        if (object_type == "project") {
            std::string name;
            ss >> name;
            if (command == "save") {
                if (projectManager->save(name, *sequencer)) {
                    std::cout << "Project '" << name << "' saved." << std::endl;
                }
            } else {
                if (projectManager->load(name, *sequencer)) {
                    std::cout << "Project '" << name << "' loaded." << std::endl;
                }
            }
            return;
        }
    }
    
    if (s == "list projects") {
        auto projects = projectManager->listProjects();
        if (projects.empty()) {
            std::cout << "No projects found in library." << std::endl;
        } else {
            std::cout << "Saved projects:" << std::endl;
            for (const auto& p : projects) std::cout << "  - " << p << std::endl;
        }
        return;
    }

    // Kit commands
    if (command == "loadkit") {
        std::string name;
        ss >> name;
        auto kit = kitManager->load(name);
        if (kit) {
            sequencer->loadKit(kit);
        }
        return;
    }

    if (command == "savekit") {
        std::string name;
        ss >> name;
        auto currentKit = sequencer->getKit();
        currentKit->setName(name);
        kitManager->save(currentKit);
        return;
    }

    if (command == "kits" || command == "listkits") {
        auto kits = kitManager->listKits();
        if (kits.empty()) {
            std::cout << "No kits found." << std::endl;
        } else {
            std::cout << "Available kits:" << std::endl;
            for (const auto& k : kits) std::cout << "  - " << k << std::endl;
        }
        return;
    }

    if (command == "newkit") {
        sequencer->loadKit(Kit::createDefaultKit());
        std::cout << "Loaded new default kit." << std::endl;
        return;
    }

    if (s == "new random kit") {
        sequencer->loadKit(createRandomKit());
        std::cout << "Generated new random kit." << std::endl;
        return;
    }

    if (s == "clear kit") {
        auto kit = sequencer->getKit();
        kit->clearInstruments();
        sequencer->loadKit(kit);
        std::cout << "Kit cleared." << std::endl;
        return;
    }

    if (command == "add") {
        std::string type, name;
        ss >> type >> name;
        if (name.empty()) name = type;
        
        static const std::vector<std::string> validTypes = {
            "simplekick", "simplesnare", "simplehat", "simpletom", "simplecymbal"
        };
        
        if (std::find(validTypes.begin(), validTypes.end(), type) != validTypes.end()) {
            auto kit = sequencer->getKit();
            kit->addInstrument(name, {type, 220.0f, 0.5f, 1.0f, 0.5f});
            sequencer->loadKit(kit); // Reload to update synths and grid
            std::cout << "Added instrument '" << name << "' of type '" << type << "'." << std::endl;
        } else {
            std::cout << "Invalid instrument type. Valid types: simplekick, simplesnare, simplehat, simpletom, simplecymbal" << std::endl;
        }
        return;
    }

    // Sequencer commands
    if (s == "new random beat" || s == "randomize" || s == "shuffle") {
        sequencer->randomize();
        std::cout << "Pattern randomized." << std::endl;
        return;
    }

    if (s == "clear" || s == "empty" || s == "reset") {
        sequencer->clear();
        std::cout << "Pattern cleared." << std::endl;
        return;
    }

    // Tempo commands
    if (s == "faster" || s == "speed up" || s == "tempo up") {
        float newBpm = sequencer->getBPM() + 10.0f;
        sequencer->setBPM(newBpm);
        std::cout << "BPM increased to " << newBpm << std::endl;
        return;
    }

    if (s == "slower" || s == "slow down" || s == "tempo down") {
        float newBpm = sequencer->getBPM() - 10.0f;
        if (newBpm < 20.0f) newBpm = 20.0f;
        sequencer->setBPM(newBpm);
        std::cout << "BPM decreased to " << newBpm << std::endl;
        return;
    }
    
    if (command == "set" || command == "set_to") {
        std::string first;
        ss >> first;
        if (first == "bpm" || first == "tempo") {
            float bpm;
            if (ss >> bpm) {
                sequencer->setBPM(bpm);
                std::cout << "BPM set to " << bpm << std::endl;
            }
        } else if (first == "steps") {
            int steps;
            if (ss >> steps) {
                sequencer->setNumSteps(steps);
                std::cout << "Number of steps set to " << steps << std::endl;
            }
        } else if (first == "kit") {
            std::string sub;
            ss >> sub;
            if (sub == "size") {
                int size;
                if (ss >> size) {
                    auto kit = sequencer->getKit();
                    int currentSize = kit->getInstruments().size();
                    if (size < currentSize) {
                        for (int i = currentSize - 1; i >= size; --i) {
                            kit->removeInstrument(i);
                        }
                    } else if (size > currentSize) {
                        auto randFloat = [](float min, float max) {
                            return min + (max - min) * (static_cast<float>(rand()) / static_cast<float>(RAND_MAX));
                        };
                        static const std::vector<std::string> types = {
                            "simplekick", "simplesnare", "simplehat", "simpletom", "simplecymbal"
                        };
                        for (int i = currentSize; i < size; ++i) {
                            std::string type = types[rand() % types.size()];
                            std::string name = type + "_" + std::to_string(i);
                            kit->addInstrument(name, {
                                type, 
                                randFloat(40.0f, 600.0f), 
                                randFloat(0.05f, 1.5f), 
                                randFloat(0.4f, 1.0f), 
                                randFloat(0.0f, 1.0f)
                            });
                        }
                    }
                    sequencer->loadKit(kit);
                    std::cout << "Kit size set to " << size << "." << std::endl;
                }
            }
        } else {
            // Assume first is instrument name: set <instrument> <param> <value>
            int channel = stringToChannelIndex(first);
            if (channel != -1) {
                std::string param;
                float value;
                if (ss >> param >> value) {
                    auto kit = sequencer->getKit();
                    auto params = kit->getParams(channel);
                    if (param == "freq" || param == "frequency") params.frequency = value;
                    else if (param == "decay") params.decay = value;
                    else if (param == "gain" || param == "vol" || param == "volume") params.gain = value;
                    else if (param == "pan") params.pan = value;
                    else {
                        std::cout << "Unknown parameter '" << param << "'. Valid: freq, decay, gain, pan" << std::endl;
                        return;
                    }
                    kit->setParams(channel, params);
                    sequencer->loadKit(kit); // Reload to update synths
                    std::cout << "Set " << param << " of " << first << " to " << value << std::endl;
                }
            } else {
                std::cout << "Unknown parameter or instrument: " << first << std::endl;
            }
        }
        return;
    }
    
    // Pattern commands
    int channel = stringToChannelIndex(command);
    if (channel != -1) {
        std::string on;
        ss >> on;
        if (on == "on") {
            std::string next;
            ss >> next;
            if (next == "every") {
                int interval;
                if (ss >> interval) {
                    for (int i = 0; i < sequencer->getNumSteps(); i += interval) {
                        sequencer->setStep(channel, i, true);
                    }
                    std::cout << "Set " << command << " on every " << interval << " steps." << std::endl;
                }
            } else {
                // Parse specific steps: "kick on 1 5 9 13"
                int step;
                try {
                    step = std::stoi(next);
                    sequencer->setStep(channel, step - 1, true);
                    while (ss >> step) {
                        sequencer->setStep(channel, step - 1, true);
                    }
                    std::cout << "Set " << command << " on specific steps." << std::endl;
                } catch (...) {}
            }
        }
    }
}
} // namespace bpm
