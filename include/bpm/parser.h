#pragma once

#include <string>
#include <memory>
#include "bpm/sequencer.h"
#include "bpm/project_manager.h"
#include "bpm/kit_manager.h"

namespace bpm {

class CommandParser {
public:
    CommandParser(std::shared_ptr<Sequencer> sequencer,
                  std::shared_ptr<ProjectManager> projectManager,
                  std::shared_ptr<KitManager> kitManager);
    void parse(const std::string& input);

private:
    std::shared_ptr<Sequencer> sequencer;
    std::shared_ptr<ProjectManager> projectManager;
    std::shared_ptr<KitManager> kitManager;
    void toLower(std::string& s);
    int stringToChannelIndex(const std::string& s);
};

} // namespace bpm
