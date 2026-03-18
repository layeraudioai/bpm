#include "bpm/project_manager.h"
#include <fstream>
#include <iostream>
#include <algorithm>

namespace bpm {

ProjectManager::ProjectManager(const std::string& projectsDir) {
    projectsPath = std::filesystem::current_path() / projectsDir;
    if (!std::filesystem::exists(projectsPath)) {
        std::filesystem::create_directory(projectsPath);
    }
}

bool ProjectManager::save(const std::string& name, const Sequencer& sequencer) {
    std::filesystem::path filePath = projectsPath / (name + ".bpm");
    std::ofstream ofs(filePath);
    if (!ofs.is_open()) {
        std::cerr << "Could not open file for saving: " << filePath << std::endl;
        return false;
    }

    // Save BPM
    ofs << sequencer.getBPM() << "\n";

    const auto& grid = sequencer.getFullGrid();
    ofs << grid.size() << "\n";
    
    for (const auto& track : grid) {
        for (int i = 0; i < sequencer.getNumSteps(); ++i) {
            ofs << (((track >> i) & 1) ? '1' : '0');
        }
        ofs << "\n";
    }

    return true;
}

bool ProjectManager::load(const std::string& name, Sequencer& sequencer) {
    std::filesystem::path filePath = projectsPath / (name + ".bpm");
    if (!std::filesystem::exists(filePath)) {
        filePath = projectsPath / name; // Try without extension
        if (!std::filesystem::exists(filePath)) {
            std::cerr << "Project not found: " << name << std::endl;
            return false;
        }
    }

    std::ifstream ifs(filePath);
    if (!ifs.is_open()) {
        std::cerr << "Could not open project file: " << filePath << std::endl;
        return false;
    }

    std::string line;
    if (!std::getline(ifs, line)) return false;
    
    try {
        sequencer.setBPM(std::stof(line));
    } catch (...) {
        std::cerr << "Invalid BPM in project file." << std::endl;
    }

    size_t numTracks = 0;
    if (std::getline(ifs, line)) numTracks = std::stoul(line);

    std::vector<uint64_t> newGrid(numTracks, 0);
    for (size_t t = 0; t < numTracks; ++t) {
        if (!std::getline(ifs, line)) break;
        for (int s = 0; s < (int)std::min((size_t)sequencer.getNumSteps(), line.size()); ++s) {
            if (line[s] == '1') {
                newGrid[t] |= (1ULL << s);
            }
        }
    }

    sequencer.setFullGrid(newGrid);
    return true;
}

std::vector<std::string> ProjectManager::listProjects() {
    std::vector<std::string> projects;
    for (const auto& entry : std::filesystem::directory_iterator(projectsPath)) {
        if (entry.is_regular_file() && entry.path().extension() == ".bpm") {
            projects.push_back(entry.path().stem().string());
        }
    }
    return projects;
}

} // namespace bpm
