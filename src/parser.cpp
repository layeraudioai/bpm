#include "bpm/parser.h"
#include <algorithm>
#ifndef __GBA__
#include <sstream>
#include <iostream>
#endif
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <map>

namespace bpm {

float noteToFrequency(const std::string& note) {
    if (note.empty()) return 0.0f;

    static const std::map<std::string, int> noteOffsets = {
        {"c", 0}, {"c#", 1}, {"db", 1}, {"d", 2}, {"d#", 3}, {"eb", 3},
        {"e", 4}, {"f", 5}, {"f#", 6}, {"gb", 6}, {"g", 7}, {"g#", 8},
        {"ab", 8}, {"a", 9}, {"a#", 10}, {"bb", 10}, {"b", 11}
    };

    std::string name;
    std::string octaveStr;
    
    size_t i = 0;
    if (i < note.length()) name += std::tolower(note[i++]);
    if (i < note.length() && (note[i] == '#' || std::tolower(note[i]) == 'b')) {
        name += std::tolower(note[i++]);
    }
    
    while (i < note.length()) {
        octaveStr += note[i++];
    }

    if (noteOffsets.find(name) == noteOffsets.end()) return 0.0f;
    
    int octave = 4;
#ifndef __GBA__
    try {
        if (!octaveStr.empty()) octave = std::stoi(octaveStr);
    } catch (...) {
        return 0.0f;
    }
#else
    if (!octaveStr.empty()) {
        char* end;
        octave = strtol(octaveStr.c_str(), &end, 10);
        if (*end != '\0') return 0.0f; // Not a valid integer
    }
#endif

    int midiNote = (octave + 1) * 12 + noteOffsets.at(name);
    return 440.0f * std::pow(2.0f, (midiNote - 69) / 12.0f);
}

CommandParser::CommandParser(std::shared_ptr<Sequencer> sequencer,
                           std::shared_ptr<ProjectManager> projectManager,
                           std::shared_ptr<KitManager> kitManager)
    : sequencer(sequencer), projectManager(projectManager), kitManager(kitManager) {}

std::shared_ptr<Kit> createRandomKit() {
    auto kit = std::make_shared<Kit>();

    auto randFloat = [](float min, float max) {
        return min + (max - min) * (static_cast<float>(rand()) / static_cast<float>(RAND_MAX));
    };

    for (uint8_t i = 0; i < (uint8_t)DrumChannel::COUNT; ++i) {
        DrumChannel ch = (DrumChannel)i;
        auto& p = (*kit)[ch];
        p.frequency = randFloat(40.0f, 1000.0f);
        p.decay = randFloat(0.05f, 1.5f);
        p.tone = randFloat(0.0f, 1.0f);
        p.noise = randFloat(0.0f, 1.0f);
        p.volume = randFloat(0.5f, 1.0f);
    }

    return kit;
}

void CommandParser::toLower(std::string& s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return std::tolower(c); });
}

int CommandParser::stringToChannelIndex(const std::string& s) {
    for (uint8_t i = 0; i < (uint8_t)DrumChannel::COUNT; ++i) {
        std::string name = Kit::channelToString((DrumChannel)i);
        std::transform(name.begin(), name.end(), name.begin(), [](unsigned char c){ return std::tolower(c); });
        if (name == s) return (int)i;
    }

    // Fuzzy matching
    for (uint8_t i = 0; i < (uint8_t)DrumChannel::COUNT; ++i) {
        std::string name = Kit::channelToString((DrumChannel)i);
        std::transform(name.begin(), name.end(), name.begin(), [](unsigned char c){ return std::tolower(c); });
        if (name.find(s) != std::string::npos) return (int)i;
    }

    return -1;
}

void CommandParser::parse(const std::string& input) {
    std::string s = input;
    toLower(s);
    std::string command;
#ifndef __GBA__
    std::stringstream ss(s);
    ss >> command;
#else
    size_t space_pos = s.find(' ');
    if (space_pos != std::string::npos) {
        command = s.substr(0, space_pos);
    } else {
        command = s;
    }
#endif

    if (command.empty()) return;

    // Check ReadOnly
    bool isMutation = (command == "add" || command == "remove" || command == "set" || 
                       command == "clear" || command == "randomize" || command == "shuffle" ||
                       command == "generate" || s.find(" on ") != std::string::npos);
    
    if (sequencer->isReadOnly() && isMutation) {
        if (s.find("set song_mode") == std::string::npos && command != "switch" && command != "pattern") {
#ifndef __GBA__
            std::cout << "Song is READONLY. Mutation commands disabled." << std::endl;
#endif
            return;
        }
    }

    // Project/Style/Structure/Song commands
    if (command == "save" || command == "load" || command == "export") {
        std::string object_type;
#ifndef __GBA__
        ss >> object_type;
        std::string name;
        ss >> name;

        if (object_type == "project") {
            if (command == "save") {
                if (projectManager->save(name, *sequencer)) std::cout << "Project '" << name << "' saved." << std::endl;
            } else if (command == "load") {
                if (projectManager->load(name, *sequencer)) std::cout << "Project '" << name << "' loaded." << std::endl;
            }
            return;
        } else if (object_type == "style") {
            if (command == "save") {
                if (projectManager->saveStyle(name, *sequencer)) std::cout << "Style '" << name << "' saved." << std::endl;
            } else if (command == "load") {
                if (projectManager->loadStyle(name, *sequencer)) std::cout << "Style '" << name << "' loaded." << std::endl;
            }
            return;
        } else if (object_type == "structure") {
            if (command == "save") {
                if (projectManager->saveStructure(name, *sequencer)) std::cout << "Structure '" << name << "' saved." << std::endl;
            } else if (command == "load") {
                if (projectManager->loadStructure(name, *sequencer)) std::cout << "Structure '" << name << "' loaded." << std::endl;
            }
            return;
        } else if (object_type == "song") {
            if (command == "export" || command == "save") {
                if (projectManager->exportSong(name, *sequencer)) std::cout << "Song exported to '" << name << ".song' (ReadOnly)." << std::endl;
            } else if (command == "load") {
                if (projectManager->loadSong(name, *sequencer)) std::cout << "Song '" << name << "' loaded (ReadOnly mode)." << std::endl;
            }
            return;
        }
#endif
    }
    
    if (s == "list projects") {
#ifndef __GBA__
        auto files = projectManager->listFiles(".bpm");
        std::cout << "Saved projects:" << std::endl;
        for (const auto& f : files) std::cout << "  - " << f << std::endl;
        return;
    }
    if (s == "list styles" || s == "styles") {
        auto files = projectManager->listFiles(".style");
        std::cout << "Available styles:" << std::endl;
        for (const auto& f : files) std::cout << "  - " << f << std::endl;
        return;
    }
    if (s == "list structures" || s == "structures") {
        auto files = projectManager->listFiles(".structure");
        std::cout << "Available structures:" << std::endl;
        for (const auto& f : files) std::cout << "  - " << f << std::endl;
        return;
    }
    if (s == "list songs" || s == "songs") {
        auto files = projectManager->listFiles(".song");
        std::cout << "Exported songs (ReadOnly):" << std::endl;
        for (const auto& f : files) std::cout << "  - " << f << std::endl;
#endif
        return;
    }

    // Kit commands
    if (command == "loadkit") {
#ifndef __GBA__
        std::string name;
        ss >> name;
        if (kitManager->loadKit(name, *sequencer)) {
             std::cout << "Kit '" << name << "' loaded." << std::endl;
        }
#endif
        return;
    }

    if (command == "savekit") {
#ifndef __GBA__
        std::string name;
        ss >> name;
        if (kitManager->saveKit(name, *(sequencer->getKit()))) {
            std::cout << "Kit '" << name << "' saved." << std::endl;
        }
#endif
        return;
    }

    if (command == "kits" || command == "listkits") {
#ifndef __GBA__
        auto kits = kitManager->listKits();
        if (kits.empty()) std::cout << "No kits found." << std::endl;
        else {
            std::cout << "Available kits:" << std::endl;
            for (const auto& k : kits) std::cout << "  - " << k << std::endl;
        }
#endif
        return;
    }

    if (command == "newkit") {
        sequencer->loadKit(Kit::createDefaultKit());
#ifndef __GBA__
        std::cout << "Loaded new default kit." << std::endl;
#endif
        return;
    }

    if (s == "new random kit") {
        sequencer->loadKit(createRandomKit());
#ifndef __GBA__
        std::cout << "Generated new random kit." << std::endl;
#endif
        return;
    }

    if (s == "clear kit") {
        sequencer->loadKit(std::make_shared<Kit>());
#ifndef __GBA__
        std::cout << "Kit cleared." << std::endl;
#endif
        return;
    }

    if (command == "add") {
#ifndef __GBA__
        std::string type;
        ss >> type;
        if (type == "pattern") {
            sequencer->addPattern();
            std::cout << "Added pattern " << sequencer->getPatternCount() - 1 << std::endl;
            return;
        }
#endif
        return;
    }

    if (command == "switch" || command == "pattern") {
#ifndef __GBA__
        std::string sub;
        if (command == "pattern") {
            if (ss >> sub) {
                if (sub == "add") {
                    sequencer->addPattern();
                    std::cout << "Added pattern " << sequencer->getPatternCount() - 1 << std::endl;
                    return;
                } else if (sub == "list") goto list_patterns;
                else {
                    try {
                        int index = std::stoi(sub);
                        sequencer->switchPattern(index);
                        std::cout << "Switched to pattern " << index << std::endl;
                        return;
                    } catch (...) {}
                }
            } else goto list_patterns;
        } else {
            ss >> sub;
            if (sub == "pattern") {
                int index;
                if (ss >> index) {
                    sequencer->switchPattern(index);
                    std::cout << "Switched to pattern " << index << std::endl;
                    return;
                }
            }
        }
#endif
    }

    if (s == "list patterns" || s == "patterns") {
#ifndef __GBA__
    list_patterns:
        std::cout << "Patterns (" << sequencer->getPatternCount() << "):" << std::endl;
        for (int i = 0; i < sequencer->getPatternCount(); ++i) {
            std::cout << (i == sequencer->getCurrentPatternIndex() ? "  * " : "    ") << i << std::endl;
        }
        const auto& arr = sequencer->getArrangement();
        if (!arr.empty()) {
            std::cout << "Arrangement: ";
            for (size_t i = 0; i < arr.size(); ++i) {
                if ((int)i == sequencer->getArrangementIndex() && sequencer->getSongMode()) std::cout << "[" << arr[i] << "] ";
                else std::cout << arr[i] << " ";
            }
            std::cout << std::endl;
        }
#endif
        return;
    }

    if (command == "remove") {
#ifndef __GBA__
        std::string type;
        ss >> type;
        if (type == "pattern") {
            int index;
            if (ss >> index) {
                sequencer->removePattern(index);
                std::cout << "Removed pattern " << index << std::endl;
                return;
            }
        }
#endif
    }

    // Sequencer commands
    if (s == "new random beat" || s == "randomize" || s == "shuffle") {
        sequencer->randomize();
#ifndef __GBA__
        std::cout << "Pattern randomized." << std::endl;
#endif
        return;
    }

    if (s == "generate full song" || s == "add random style" || s == "generate random style") {
        bool keepKit = (s.find("style") != std::string::npos);
        int styleRoll = rand() % 100;
        std::string styleName = "Pop";
        int numPatterns = 4;
        std::vector<int> densities;
        std::vector<int> arrangement;

        if (styleRoll < 80) {
            styleName = "Pop";
            numPatterns = 4;
            densities = {5, 12, 22, 8};
            arrangement = {0, 1, 1, 2, 2, 1, 2, 2, 3}; 
        } else if (styleRoll < 90) {
            styleName = "Club/Techno";
            numPatterns = 4;
            densities = {5, 15, 30, 10};
            arrangement = {0, 0, 1, 1, 2, 2, 2, 2, 1, 1, 3, 3};
        } else if (styleRoll < 95) {
            styleName = "Minimalist";
            numPatterns = 3;
            densities = {2, 5, 8};
            arrangement = {0, 0, 1, 1, 2, 2, 1, 1, 0, 0};
        } else {
            styleName = "Experimental/Chaos";
            numPatterns = 2 + (rand() % 6);
            for (int i = 0; i < numPatterns; ++i) densities.push_back(1 + (rand() % 40));
            int arrLen = 4 + (rand() % 10);
            for (int i = 0; i < arrLen; ++i) arrangement.push_back(rand() % numPatterns);
        }

        if (!keepKit) sequencer->loadKit(createRandomKit());
        
        while (sequencer->getPatternCount() > 1) sequencer->removePattern(0);
        sequencer->switchPattern(0);
        sequencer->clear();
        
        for (int i = 1; i < numPatterns; ++i) sequencer->addPattern();
        
        for (int p = 0; p < numPatterns; ++p) {
            sequencer->switchPattern(p);
            int d = densities[p];
            
            for (uint8_t t = 0; t < (uint8_t)DrumChannel::COUNT; ++t) {
                std::string name = Kit::channelToString((DrumChannel)t);
                std::transform(name.begin(), name.end(), name.begin(), [](unsigned char c){ return std::tolower(c); });

                if (name.find("kick") != std::string::npos) {
                    for (int step = 0; step < sequencer->getNumSteps(); step += 4) {
                        if (rand() % 100 < (d * 3)) sequencer->setStep(t, step, true);
                    }
                } else if (name.find("snare") != std::string::npos) {
                    for (int step = 4; step < sequencer->getNumSteps(); step += 8) {
                        if (rand() % 100 < (d * 2)) sequencer->setStep(t, step, true);
                    }
                } else if (name.find("hat") != std::string::npos) {
                    int interval = (d > 20) ? 2 : 4;
                    for (int step = 0; step < sequencer->getNumSteps(); step += interval) {
                        if (rand() % 100 < 70) sequencer->setStep(t, step, true);
                    }
                } else {
                    for (int step = 0; step < sequencer->getNumSteps(); ++step) {
                        if (rand() % 100 < d) sequencer->setStep(t, step, true);
                    }
                }
            }
        }
        
        sequencer->setArrangement(arrangement);
        sequencer->switchPattern(arrangement[0]);
        sequencer->setSongMode(true);
        #ifndef __GBA__
        std::cout << "Generated " << styleName << " (" << arrangement.size() << " sections)." << std::endl;
        #endif
        return;
    }

    if (s == "generate random arrangement" || s == "new random arrangement") {
        int numPatterns = sequencer->getPatternCount();
        if (numPatterns == 0) return;

        int styleRoll = rand() % 100;
        std::vector<int> arrangement;
        if (styleRoll < 70 && numPatterns >= 3) {
            arrangement = {0, 1, 1, 2, 2};
            if (numPatterns > 1) arrangement.push_back(1);
            if (numPatterns > 2) { arrangement.push_back(2); arrangement.push_back(2); }
            arrangement.push_back(numPatterns > 3 ? 3 : 0);
        } else if (styleRoll < 90) {
            for (int i = 0; i < numPatterns; ++i) {
                int reps = 1 + (rand() % 3);
                for (int r = 0; reps > r; ++r) arrangement.push_back(i);
            }
        } else {
            int len = 4 + (rand() % 12);
            for (int i = 0; i < len; ++i) arrangement.push_back(rand() % numPatterns);
        }

        sequencer->setArrangement(arrangement);
        sequencer->switchPattern(arrangement[0]);
        return;
    }

    if (s == "clear" || s == "empty" || s == "reset") {
        sequencer->clear();
        return;
    }

    if (command == "set") {
        #ifndef __GBA__
        std::string first;
        ss >> first;
        if (first == "bpm" || first == "tempo") {
            float bpm;
            if (ss >> bpm) sequencer->setBPM(bpm);
        } else if (first == "steps") {
            int steps;
            if (ss >> steps) sequencer->setNumSteps(steps);
        } else if (first == "song_mode") {
            std::string state;
            if (ss >> state) {
                bool enabled = (state == "on" || state == "1" || state == "true" || state == "yes");
                sequencer->setSongMode(enabled);
            }
        } else {
            int channel = stringToChannelIndex(first);
            if (channel != -1) {
                std::string param, valueStr;
                if (ss >> param >> valueStr) {
                    auto kit = sequencer->getKit();
                    auto& p = (*kit)[(DrumChannel)channel];
                    float value = 0.0f;
                    if ((param == "freq" || param == "frequency") && std::isalpha(valueStr[0])) value = noteToFrequency(valueStr);
                    else {
                        value = std::atof(valueStr.c_str());
                    }
                    if (param == "freq" || param == "frequency") p.frequency = value;
                    else if (param == "decay") p.decay = value;
                    else if (param == "tone") p.tone = value;
                    else if (param == "noise") p.noise = value;
                    else if (param == "volume" || param == "gain") p.volume = value;
                    else return;
                    sequencer->loadKit(kit);
                }
            }
        }
        #endif
        return;
    }
    
    int channel = stringToChannelIndex(command);
    if (channel != -1) {
        #ifndef __GBA__
        std::string on;
        if (ss >> on && on == "on") {
            int step;
            while (ss >> step) sequencer->setStep(channel, step - 1, true);
        }
        #endif
    }
}

} // namespace bpm
