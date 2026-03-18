#pragma once

#include <string>
#include <vector>
#include <memory>
#include <filesystem>
#include "bpm/sequencer.h"

namespace bpm {

class ProjectManager {
public:
    ProjectManager(const std::string& projectsDir = "projects");

    bool save(const std::string& name, const Sequencer& sequencer);
    bool load(const std::string& name, Sequencer& sequencer);
    
    bool saveStyle(const std::string& name, const Sequencer& sequencer);
    bool loadStyle(const std::string& name, Sequencer& sequencer);

    bool saveStructure(const std::string& name, const Sequencer& sequencer);
    bool loadStructure(const std::string& name, Sequencer& sequencer);

    bool exportSong(const std::string& name, const Sequencer& sequencer);
    bool loadSong(const std::string& name, Sequencer& sequencer);

    std::vector<std::string> listFiles(const std::string& extension);
    std::vector<std::string> listProjects();

private:
    std::filesystem::path projectsPath;
};

} // namespace bpm
