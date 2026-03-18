#include "bpm/project_manager.h"
#ifndef __GBA__
#include <fstream>
#include <iostream>
#include <algorithm>
#include <sstream>
#endif

namespace bpm {

ProjectManager::ProjectManager(const std::string& projectsDir) {
#ifndef __GBA__
    projectsPath = std::filesystem::current_path() / projectsDir;
    if (!std::filesystem::exists(projectsPath)) {
        std::filesystem::create_directory(projectsPath);
    }
#endif
}

// Helper to save kit data to stream
void saveKitToStream(std::ostream& os, std::shared_ptr<Kit> kit) {
#ifndef __GBA__
    if (!kit) return;
    os << "[kit]\n";
    os << "name=" << kit->getName() << "\n";
    const auto& instruments = kit->getInstruments();
    os << instruments.size() << "\n";
    for (const auto& inst : instruments) {
        os << inst.name << "," << inst.params.type << "," 
           << inst.params.frequency << "," << inst.params.decay << ","
           << inst.params.gain << "," << inst.params.pan << "\n";
    }
#endif
}

// Helper to load kit data from stream
void loadKitFromStream(std::istream& is, Sequencer& sequencer) {
#ifndef __GBA__
    std::string line;
    if (!std::getline(is, line) || line != "[kit]") return;
    
    std::string kitName = "unknown";
    if (std::getline(is, line) && line.find("name=") == 0) {
        kitName = line.substr(5);
    }

    auto kit = std::make_shared<Kit>(kitName);
    size_t numInst = 0;
    if (std::getline(is, line)) numInst = std::stoul(line);

    for (size_t i = 0; i < numInst; ++i) {
        if (!std::getline(is, line)) break;
        std::stringstream ss(line);
        std::string name, type, freq, decay, gain, pan;
        std::getline(ss, name, ',');
        std::getline(ss, type, ',');
        std::getline(ss, freq, ',');
        std::getline(ss, decay, ',');
        std::getline(ss, gain, ',');
        std::getline(ss, pan, ',');
        
        DrumSynthParams p;
        p.type = type;
        p.frequency = std::stof(freq);
        p.decay = std::stof(decay);
        p.gain = std::stof(gain);
        p.pan = std::stof(pan);
        kit->addInstrument(name, p);
    }
    sequencer.loadKit(kit);
#endif
}

// Helper to save arrangement
void saveArrangementToStream(std::ostream& os, const Sequencer& sequencer) {
#ifndef __GBA__
    const auto& arr = sequencer.getArrangement();
    os << "[arrangement]\n";
    os << arr.size() << "\n";
    for (size_t i = 0; i < arr.size(); ++i) {
        os << arr[i] << (i == arr.size() - 1 ? "" : ",");
    }
    os << "\n";
#endif
}

// Helper to load arrangement
void loadArrangementFromStream(std::istream& is, Sequencer& sequencer) {
#ifndef __GBA__
    std::string line;
    if (!std::getline(is, line) || line != "[arrangement]") return;
    
    size_t size = 0;
    if (std::getline(is, line)) size = std::stoul(line);
    
    std::vector<int> arr;
    if (size > 0 && std::getline(is, line)) {
        std::stringstream ss(line);
        std::string val;
        while (std::getline(ss, val, ',')) {
            arr.push_back(std::stoi(val));
        }
    }
    sequencer.setArrangement(arr);
#endif
}

// Helper to save patterns
void savePatternsToStream(std::ostream& os, const Sequencer& sequencer) {
#ifndef __GBA__
    const auto& allPatterns = sequencer.getAllPatterns();
    os << "[patterns]\n";
    os << allPatterns.size() << "\n";
    for (const auto& grid : allPatterns) {
        os << grid.size() << "\n";
        for (const auto& track : grid) {
            for (int i = 0; i < sequencer.getNumSteps(); ++i) {
                os << (((track >> i) & 1) ? '1' : '0');
            }
            os << "\n";
        }
    }
#endif
}

// Helper to load patterns
void loadPatternsFromStream(std::istream& is, Sequencer& sequencer) {
#ifndef __GBA__
    std::string line;
    if (!std::getline(is, line) || line != "[patterns]") return;
    
    size_t numPatterns = 0;
    if (std::getline(is, line)) numPatterns = std::stoul(line);

    std::vector<std::vector<uint64_t>> allPatterns;
    for (size_t p = 0; p < numPatterns; ++p) {
        size_t numTracks = 0;
        if (!std::getline(is, line)) break;
        numTracks = std::stoul(line);
        std::vector<uint64_t> grid(numTracks, 0);
        for (size_t t = 0; t < numTracks; ++t) {
            if (!std::getline(is, line)) break;
            for (int s = 0; s < (int)std::min((size_t)sequencer.getNumSteps(), line.size()); ++s) {
                if (line[s] == '1') grid[t] |= (1ULL << s);
            }
        }
        allPatterns.push_back(grid);
    }
    sequencer.setAllPatterns(allPatterns);
#endif
}

bool ProjectManager::save(const std::string& name, const Sequencer& sequencer) {
#ifndef __GBA__
    std::filesystem::path filePath = projectsPath / (name + ".bpm");
    std::ofstream ofs(filePath);
    if (!ofs.is_open()) return false;

    ofs << sequencer.getBPM() << "\n";
    saveKitToStream(ofs, sequencer.getKit());
    saveArrangementToStream(ofs, sequencer);
    savePatternsToStream(ofs, sequencer);
    return true;
#else
    return false;
#endif
}

bool ProjectManager::load(const std::string& name, Sequencer& sequencer) {
#ifndef __GBA__
    std::filesystem::path filePath = projectsPath / (name + ".bpm");
    if (!std::filesystem::exists(filePath)) return false;
    std::ifstream ifs(filePath);
    if (!ifs.is_open()) return false;

    std::string line;
    if (!std::getline(ifs, line)) return false;
    sequencer.setBPM(std::stof(line));
    sequencer.setReadOnly(false);

    loadKitFromStream(ifs, sequencer);
    loadArrangementFromStream(ifs, sequencer);
    loadPatternsFromStream(ifs, sequencer);
    return true;
#else
    return false;
#endif
}

bool ProjectManager::saveStyle(const std::string& name, const Sequencer& sequencer) {
#ifndef __GBA__
    std::filesystem::path filePath = projectsPath / (name + ".style");
    std::ofstream ofs(filePath);
    if (!ofs.is_open()) return false;

    saveArrangementToStream(ofs, sequencer);
    savePatternsToStream(ofs, sequencer);
    return true;
#else
    return false;
#endif
}

bool ProjectManager::loadStyle(const std::string& name, Sequencer& sequencer) {
#ifndef __GBA__
    std::filesystem::path filePath = projectsPath / (name + ".style");
    if (!std::filesystem::exists(filePath)) return false;
    std::ifstream ifs(filePath);
    if (!ifs.is_open()) return false;

    loadArrangementFromStream(ifs, sequencer);
    loadPatternsFromStream(ifs, sequencer);
    return true;
#else
    return false;
#endif
}

bool ProjectManager::saveStructure(const std::string& name, const Sequencer& sequencer) {
#ifndef __GBA__
    std::filesystem::path filePath = projectsPath / (name + ".structure");
    std::ofstream ofs(filePath);
    if (!ofs.is_open()) return false;

    saveArrangementToStream(ofs, sequencer);
    return true;
#else
    return false;
#endif
}

bool ProjectManager::loadStructure(const std::string& name, Sequencer& sequencer) {
#ifndef __GBA__
    std::filesystem::path filePath = projectsPath / (name + ".structure");
    if (!std::filesystem::exists(filePath)) return false;
    std::ifstream ifs(filePath);
    if (!ifs.is_open()) return false;

    loadArrangementFromStream(ifs, sequencer);
    return true;
#else
    return false;
#endif
}

bool ProjectManager::exportSong(const std::string& name, const Sequencer& sequencer) {
#ifndef __GBA__
    std::filesystem::path filePath = projectsPath / (name + ".song");
    std::ofstream ofs(filePath);
    if (!ofs.is_open()) return false;

    ofs << "READONLY\n";
    ofs << sequencer.getBPM() << "\n";
    saveKitToStream(ofs, sequencer.getKit());
    saveArrangementToStream(ofs, sequencer);
    savePatternsToStream(ofs, sequencer);
    return true;
#else
    return false;
#endif
}

bool ProjectManager::loadSong(const std::string& name, Sequencer& sequencer) {
#ifndef __GBA__
    std::filesystem::path filePath = projectsPath / (name + ".song");
    if (!std::filesystem::exists(filePath)) return false;
    std::ifstream ifs(filePath);
    if (!ifs.is_open()) return false;

    std::string line;
    if (!std::getline(ifs, line) || line != "READONLY") return false;
    
    if (!std::getline(ifs, line)) return false;
    sequencer.setBPM(std::stof(line));
    sequencer.setReadOnly(true);

    loadKitFromStream(ifs, sequencer);
    loadArrangementFromStream(ifs, sequencer);
    loadPatternsFromStream(ifs, sequencer);
    return true;
#else
    return false;
#endif
}

std::vector<std::string> ProjectManager::listFiles(const std::string& extension) {
    std::vector<std::string> files;
#ifndef __GBA__
    for (const auto& entry : std::filesystem::directory_iterator(projectsPath)) {
        if (entry.is_regular_file() && entry.path().extension() == extension) {
            files.push_back(entry.path().stem().string());
        }
    }
#endif
    return files;
}

std::vector<std::string> ProjectManager::listProjects() {
    return listFiles(".bpm");
}

} // namespace bpm
