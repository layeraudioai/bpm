#pragma once

#include <string>
#include <memory>
#include "bpm/sequencer.h"
#include "bpm/project_manager.h"

namespace bpm {

class CommandParser {
public:
    CommandParser(std::shared_ptr<Sequencer> sequencer, std::shared_ptr<ProjectManager> projectManager);
    void parse(const std::string& input);

private:
    std::shared_ptr<Sequencer> sequencer;
    std::shared_ptr<ProjectManager> projectManager;
    void toLower(std::string& s);
    DrumChannel stringToChannel(const std::string& s);
};

} // namespace bpm
