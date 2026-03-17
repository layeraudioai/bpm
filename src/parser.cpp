#include "bpm/parser.h"
#include <algorithm>
#include <sstream>
#include <vector>
#include <iostream>

namespace bpm {

CommandParser::CommandParser(std::shared_ptr<Sequencer> sequencer, std::shared_ptr<ProjectManager> projectManager)
    : sequencer(sequencer), projectManager(projectManager) {}

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

    if (s.find("save project ") == 0) {
        std::string name = input.substr(13); 
        if (projectManager->save(name, *sequencer)) {
            std::cout << "Project '" << name << "' saved." << std::endl;
        }
        return;
    }

    if (s.find("load project ") == 0) {
        std::string name = input.substr(13);
        if (projectManager->load(name, *sequencer)) {
            std::cout << "Project '" << name << "' loaded." << std::endl;
        }
        return;
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

    if (s.find("bpm") || s.find("set tempo") == 0) {
        std::stringstream ss(s);
        std::string token;
        ss >> token; // "bpm" or "set"
        if (token == "set") ss >> token; // skip "tempo"
        int bpm;
        if (ss >> bpm) {
            sequencer->setBPM(static_cast<float>(bpm));
            std::cout << "BPM set to " << bpm << std::endl;
            return;
        }
    }
    
    // Try to parse "kick on 1 5 9 13" or "kick on every 4"
    std::stringstream ss(s);
    std::string token;
    ss >> token;

    DrumChannel channel = stringToChannel(token);
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
                    std::cout << "Set " << token << " on every " << interval << " steps." << std::endl;
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
                    std::cout << "Set " << token << " on specific steps." << std::endl;
                } catch (...) {}
            }
        }
    } else {
        std::cout << "Unknown command: " << s << std::endl;
    }
}

} // namespace bpm
