#include "bpm/parser.h"
#include <algorithm>
#include <sstream>
#include <vector>
#include <iostream>
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
    kit->addInstrument("Bass",         {"simplebeep", 80.0f, 0.4f, 0.8f, 0.5f});
    kit->addInstrument("Lead",         {"simplebeep", 440.0f, 0.2f, 0.5f, 0.4f});
    kit->addInstrument("Arp",          {"simplebeep", 880.0f, 0.1f, 0.4f, 0.6f});

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

    // Check ReadOnly
    bool isMutation = (command == "add" || command == "remove" || command == "set" || 
                       command == "clear" || command == "randomize" || command == "shuffle" ||
                       command == "generate" || s.find(" on ") != std::string::npos);
    
    if (sequencer->isReadOnly() && isMutation) {
        if (s.find("set song_mode") == std::string::npos && command != "switch" && command != "pattern") {
            std::cout << "Song is READONLY. Mutation commands disabled." << std::endl;
            return;
        }
    }

    // Project/Style/Structure/Song commands
    if (command == "save" || command == "load" || command == "export") {
        std::string object_type;
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
    }
    
    if (s == "list projects") {
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
        return;
    }

    // Kit commands
    if (command == "loadkit") {
        std::string name;
        ss >> name;
        auto kit = kitManager->load(name);
        if (kit) sequencer->loadKit(kit);
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
        if (kits.empty()) std::cout << "No kits found." << std::endl;
        else {
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
        
        if (type == "pattern") {
            sequencer->addPattern();
            std::cout << "Added pattern " << sequencer->getPatternCount() - 1 << std::endl;
            return;
        }

        if (name.empty()) name = type;
        static const std::vector<std::string> validTypes = {
            "simplekick", "simplesnare", "simplehat", "simpletom", "simplecymbal", "simplebeep"
        };
        
        if (std::find(validTypes.begin(), validTypes.end(), type) != validTypes.end()) {
            auto kit = sequencer->getKit();
            kit->addInstrument(name, {type, 220.0f, 0.5f, 1.0f, 0.5f});
            sequencer->loadKit(kit);
            std::cout << "Added instrument '" << name << "' of type '" << type << "'." << std::endl;
        } else {
            std::cout << "Invalid instrument type." << std::endl;
        }
        return;
    }

    if (command == "switch" || command == "pattern") {
        std::string sub;
        if (command == "pattern") {
            if (ss >> sub) {
                if (sub == "add") {
                    sequencer->addPattern();
                    std::cout << "Added pattern " << sequencer->getPatternCount() - 1 << std::endl;
                    return;
                } else if (sub == "list") goto list_patterns;
                else {
#ifndef __GBA__
                    try {
                        int index = std::stoi(sub);
                        sequencer->switchPattern(index);
                        std::cout << "Switched to pattern " << index << std::endl;
                        return;
                    } catch (...) {}
#else
                    int index = atoi(sub.c_str());
                    sequencer->switchPattern(index);
                    return;
#endif
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
    }

    if (s == "list patterns" || s == "patterns") {
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
        return;
    }

    if (command == "remove") {
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
    }

    // Sequencer commands
    if (s == "new random beat" || s == "randomize" || s == "shuffle") {
        sequencer->randomize();
        std::cout << "Pattern randomized." << std::endl;
        return;
    }

    if (s == "generate new beep pattern" || s == "generate new melody" || s == "generate melody") {
        auto kit = sequencer->getKit();
        auto instruments = kit->getInstruments();
        std::vector<int> beepChannels;
        for (size_t i = 0; i < instruments.size(); ++i) {
            if (instruments[i].params.type == "simplebeep") beepChannels.push_back((int)i);
        }
        if (beepChannels.empty()) {
            std::cout << "No beep instruments found." << std::endl;
            return;
        }

        static const std::vector<int> scale = {0, 2, 4, 5, 7, 9, 11}; // Major scale
        int root = rand() % 12;

        for (int channel : beepChannels) {
            std::string name = instruments[channel].name;
            std::transform(name.begin(), name.end(), name.begin(), [](unsigned char c){ return std::tolower(c); });
            
            for (int i = 0; i < sequencer->getNumSteps(); ++i) sequencer->setStep(channel, i, false);
            
            auto setFreq = [&](int noteIdx, int octave) {
                int noteMidi = (octave + 1) * 12 + root + scale[noteIdx % scale.size()];
                float freq = 440.0f * std::pow(2.0f, (noteMidi - 69) / 12.0f);
                auto params = kit->getParams(channel);
                params.frequency = freq;
                kit->setParams(channel, params);
                sequencer->loadKit(kit);
            };

            if (name.find("bass") != std::string::npos) {
                // Bassline: often on 1, 5, 9, 13 or similar
                int octave = 1 + (rand() % 2);
                setFreq(0, octave); // Stay on root for now
                for (int i = 0; i < sequencer->getNumSteps(); i += 4) {
                    if (rand() % 100 < 80) sequencer->setStep(channel, i, true);
                }
            } else if (name.find("arp") != std::string::npos) {
                // Arpeggio: 8th or 16th notes
                int octave = 4 + (rand() % 2);
                int interval = (rand() % 2 == 0) ? 2 : 4;
                for (int i = 0; i < sequencer->getNumSteps(); i += interval) {
                    setFreq(rand() % scale.size(), octave);
                    if (rand() % 100 < 70) sequencer->setStep(channel, i, true);
                }
            } else {
                // Lead/Default: Rhythmic riffs
                int octave = 3 + (rand() % 2);
                int density = 4 + (rand() % 12);
                for (int i = 0; i < density; ++i) {
                    int step = rand() % sequencer->getNumSteps();
                    setFreq(rand() % scale.size(), octave);
                    sequencer->setStep(channel, step, true);
                }
            }
        }
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
            arrangement = {0, 1, 1, 2, 2, 1, 2, 2, 3}; // Intro, V, V, C, C, V, C, C, Outro
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
        
        // Clear patterns safely: keep at least one and clear it, then add others
        while (sequencer->getPatternCount() > 1) sequencer->removePattern(0);
        sequencer->switchPattern(0);
        sequencer->clear();
        
        for (int i = 1; i < numPatterns; ++i) sequencer->addPattern();
        
        for (int p = 0; p < numPatterns; ++p) {
            sequencer->switchPattern(p);
            // sequencer->clear(); // Already clear if new, or cleared above for p=0
            int d = densities[p];
            auto instruments = sequencer->getKit()->getInstruments();
            
            for (size_t t = 0; t < instruments.size(); ++t) {
                std::string name = instruments[t].name;
                std::string type = instruments[t].params.type;
                std::transform(name.begin(), name.end(), name.begin(), [](unsigned char c){ return std::tolower(c); });

                if (type == "simplebeep") continue; // Handled by generate melody

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
            
            bool hasBeep = false;
            for (const auto& inst : instruments) {
                if (inst.params.type == "simplebeep") {
                    hasBeep = true;
                    break;
                }
            }
            if (hasBeep) parse("generate melody");
        }
        
        sequencer->setArrangement(arrangement);
        sequencer->switchPattern(arrangement[0]);
        sequencer->setSongMode(true);
        std::cout << "Generated " << styleName << " (" << arrangement.size() << " sections)." << std::endl;
        return;
    }

    if (s == "generate random arrangement" || s == "new random arrangement") {
        int numPatterns = sequencer->getPatternCount();
        if (numPatterns == 0) {
            std::cout << "No patterns available to arrange." << std::endl;
            return;
        }

        int styleRoll = rand() % 100;
        std::vector<int> arrangement;
        std::string styleName;

        if (styleRoll < 70 && numPatterns >= 3) {
            styleName = "Pop";
            // Intro(0), V(1), V(1), C(2), C(2), V(1), C(2), C(2), Outro(3 or 0)
            arrangement = {0, 1, 1, 2, 2};
            if (numPatterns > 1) arrangement.push_back(1);
            if (numPatterns > 2) { arrangement.push_back(2); arrangement.push_back(2); }
            arrangement.push_back(numPatterns > 3 ? 3 : 0);
        } else if (styleRoll < 90) {
            styleName = "Linear/Club";
            // Play each pattern a few times
            for (int i = 0; i < numPatterns; ++i) {
                int reps = 1 + (rand() % 3);
                for (int r = 0; reps > r; ++r) arrangement.push_back(i);
            }
        } else {
            styleName = "Random Chaos";
            int len = 4 + (rand() % 12);
            for (int i = 0; i < len; ++i) arrangement.push_back(rand() % numPatterns);
        }

        sequencer->setArrangement(arrangement);
        std::cout << "Generated random '" << styleName << "' arrangement using " << numPatterns << " patterns." << std::endl;
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
        return;
    }
    if (s == "slower" || s == "slow down" || s == "tempo down") {
        float newBpm = sequencer->getBPM() - 10.0f;
        if (newBpm < 20.0f) newBpm = 20.0f;
        sequencer->setBPM(newBpm);
        return;
    }
    
    if (command == "set" || command == "set_to") {
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
                std::cout << "Song mode " << (enabled ? "ON" : "OFF") << std::endl;
            }
        } else if (first == "arrangement") {
            std::vector<int> arr;
            int p;
            while (ss >> p) arr.push_back(p);
            sequencer->setArrangement(arr);
            std::cout << "Arrangement updated." << std::endl;
        } else if (first == "kit") {
            std::string sub;
            ss >> sub;
            if (sub == "size") {
                int size;
                if (ss >> size) {
                    auto kit = sequencer->getKit();
                    int currentSize = kit->getInstruments().size();
                    if (size < currentSize) {
                        for (int i = currentSize - 1; i >= size; --i) kit->removeInstrument(i);
                    } else if (size > currentSize) {
                        auto randFloat = [](float min, float max) { return min + (max - min) * (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)); };
                        static const std::vector<std::string> types = {"simplekick", "simplesnare", "simplehat", "simpletom", "simplecymbal", "simplebeep"};
                        for (int i = currentSize; i < size; ++i) {
                            std::string type = types[rand() % types.size()];
                            kit->addInstrument(type + "_" + std::to_string(i), {type, randFloat(40.0f, 600.0f), randFloat(0.05f, 1.5f), randFloat(0.4f, 1.0f), randFloat(0.0f, 1.0f)});
                        }
                    }
                    sequencer->loadKit(kit);
                }
            }
        } else {
            int channel = stringToChannelIndex(first);
            if (channel != -1) {
                std::string param, valueStr;
                if (ss >> param >> valueStr) {
                    auto kit = sequencer->getKit();
                    auto params = kit->getParams(channel);
                    float value = 0.0f;
                    if ((param == "freq" || param == "frequency") && std::isalpha(valueStr[0])) value = noteToFrequency(valueStr);
                    else {
#ifndef __GBA__
                        try { value = std::stof(valueStr); } catch (...) { return; }
#else
                        char* end;
                        value = strtof(valueStr.c_str(), &end);
                        if (*end != '\0' && !isspace(*end)) return;
#endif
                    }
                    if (param == "freq" || param == "frequency") params.frequency = value;
                    else if (param == "decay") params.decay = value;
                    else if (param == "gain" || param == "vol" || param == "volume") params.gain = value;
                    else if (param == "pan") params.pan = value;
                    else return;
                    kit->setParams(channel, params);
                    sequencer->loadKit(kit);
                }
            }
        }
        return;
    }
    
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
                    for (int i = 0; i < sequencer->getNumSteps(); i += interval) sequencer->setStep(channel, i, true);
                }
            } else {
#ifndef __GBA__
                try {
                    int step = std::stoi(next);
                    sequencer->setStep(channel, step - 1, true);
                    while (ss >> step) sequencer->setStep(channel, step - 1, true);
                } catch (...) {}
#else
                int step = atoi(next.c_str());
                sequencer->setStep(channel, step - 1, true);
                while (ss >> step) sequencer->setStep(channel, step - 1, true);
#endif
            }
        }
    }
}
} // namespace bpm
