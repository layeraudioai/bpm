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
        return min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (max - min)));
    };

    kit->setParams(DrumChannel::KickLeft,     {"simplekick", randFloat(40.0f, 150.0f), randFloat(0.3f, 0.8f)});
    kit->setParams(DrumChannel::KickRight,    {"simplekick", randFloat(40.0f, 150.0f), randFloat(0.3f, 0.8f)});
    kit->setParams(DrumChannel::SnareClosed,  {"simplesnare", 0.0f, randFloat(0.1f, 0.4f)});
    kit->setParams(DrumChannel::SnareOpen,    {"simplesnare", 0.0f, randFloat(0.2f, 0.6f)});
    kit->setParams(DrumChannel::SnareRim,     {"simplesnare", 0.0f, randFloat(0.05f, 0.2f)});
    kit->setParams(DrumChannel::ClosedHat,    {"simplehat", 0.0f, randFloat(0.05f, 0.15f)});
    kit->setParams(DrumChannel::OpenHat,      {"simplehat", 0.0f, randFloat(0.3f, 0.8f)});
    kit->setParams(DrumChannel::OpeningHat,   {"simplehat", 0.0f, randFloat(0.2f, 0.5f)});
    kit->setParams(DrumChannel::Crash,        {"simplecymbal", 0.0f, randFloat(1.0f, 2.5f)});
    kit->setParams(DrumChannel::Ride,         {"simplecymbal", 0.0f, randFloat(1.5f, 3.0f)});
    kit->setParams(DrumChannel::SmallTom,     {"simpletom", randFloat(250.0f, 400.0f), randFloat(0.2f, 0.5f)});
    kit->setParams(DrumChannel::MidTom,       {"simpletom", randFloat(150.0f, 300.0f), randFloat(0.3f, 0.6f)});
    kit->setParams(DrumChannel::HighTom,      {"simpletom", randFloat(100.0f, 250.0f), randFloat(0.4f, 0.7f)});

    return kit;
}

void CommandParser::toLower(std::string& s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return std::tolower(c); });
}

DrumChannel CommandParser::stringToChannel(const std::string& s) {
    if (s == "kick" || s == "kickleft") return DrumChannel::KickLeft;
    if (s == "kickright") return DrumChannel::KickRight;
    if (s == "snare" || s == "snareopen") return DrumChannel::SnareOpen;
    if (s == "snareclosed") return DrumChannel::SnareClosed;
    if (s == "rim") return DrumChannel::SnareRim;
    if (s == "hat" || s == "closedhat") return DrumChannel::ClosedHat;
    if (s == "openhat") return DrumChannel::OpenHat;
    if (s == "openinghat") return DrumChannel::OpeningHat;
    if (s == "crash") return DrumChannel::Crash;
    if (s == "ride") return DrumChannel::Ride;
    if (s == "smalltom") return DrumChannel::SmallTom;
    if (s == "midtom") return DrumChannel::MidTom;
    if (s == "hightom") return DrumChannel::HighTom;
    return DrumChannel::Count;
}

void CommandParser::parse(const std::string& input) {
    std::string s = input;
    toLower(s);
    std::stringstream ss(s);
    std::string command;
    ss >> command;

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
        std::string param;
        ss >> param;
        if (param == "bpm" || param == "tempo") {
            float bpm;
            if (ss >> bpm) {
                sequencer->setBPM(bpm);
                std::cout << "BPM set to " << bpm << std::endl;
            }
        }
        return;
    }
    
    // Pattern commands
    DrumChannel channel = stringToChannel(command);
    if (channel != DrumChannel::Count) {
        std::string on;
        ss >> on;
        if (on == "on") {
            std::string next;
            ss >> next;
            if (next == "every") {
                int interval;
                if (ss >> interval) {
                    for (int i = 0; i < Sequencer::NumSteps; i += interval) {
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
