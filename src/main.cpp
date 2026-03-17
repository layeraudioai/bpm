#include <iostream>
#include <memory>
#include <thread>
#include <chrono>
#include <string>
#include <vector>
#include "bpm/sequencer.h"
#include "bpm/audio_engine.h"
#include "bpm/parser.h"
#include "bpm/project_manager.h"
#include "bpm/kit_manager.h"

using namespace bpm;

void printHelp() {
    std::cout << "Usage: bpm [options]" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -n            Start a new project" << std::endl;
    std::cout << "  -l <name>     Load a project on boot" << std::endl;
    std::cout << "  -h            Show this help" << std::endl;
}

int main(int argc, char** argv) {
    auto sequencer = std::make_shared<Sequencer>();
    auto projectManager = std::make_shared<ProjectManager>();
    auto kitManager = std::make_shared<KitManager>();
    
    std::string loadOnBoot = "";
    bool newProject = false;

    // Basic CLI parsing
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-n") {
            newProject = true;
        } else if (arg == "-l" && i + 1 < argc) {
            loadOnBoot = argv[++i];
        } else if (arg == "-h") {
            printHelp();
            return 0;
        }
    }

    // Interactive startup if no flags provided
    if (!newProject && loadOnBoot.empty()) {
        std::cout << "Welcome to BPM Beat Procedure Machine!" << std::endl;
        std::cout << "[n] New project" << std::endl;
        std::cout << "[l] Load project" << std::endl;
        std::cout << "Choice: " << std::flush;
        
        std::string choice;
        std::getline(std::cin, choice);
        if (choice == "l") {
            auto projects = projectManager->listProjects();
            if (projects.empty()) {
                std::cout << "No projects found. Starting new project..." << std::endl;
            } else {
                std::cout << "Available projects:" << std::endl;
                for (const auto& p : projects) std::cout << "  - " << p << std::endl;
                std::cout << "Project name to load: " << std::flush;
                std::getline(std::cin, loadOnBoot);
            }
        }
    }

    if (!loadOnBoot.empty()) {
        if (!projectManager->load(loadOnBoot, *sequencer)) {
            std::cout << "Failed to load project '" << loadOnBoot << "'. Starting empty." << std::endl;
        } else {
            std::cout << "Loaded project '" << loadOnBoot << "'." << std::endl;
        }
    }

    std::cout << "------------------------------------------" << std::endl;
    std::cout << "      BPM Beat Procedure Machine         " << std::endl;
    std::cout << "------------------------------------------" << std::endl;
    std::cout << "Commands examples:" << std::endl;
    std::cout << "  - 'save project mybeat'" << std::endl;
    std::cout << "  - 'load project mybeat'" << std::endl;
    std::cout << "  - 'list projects'" << std::endl;
    std::cout << "  - 'savekit mykit'" << std::endl;
    std::cout << "  - 'loadkit mykit'" << std::endl;
    std::cout << "  - 'kits'" << std::endl;
    std::cout << "  - 'newkit'" << std::endl;
    std::cout << "  - 'new random kit'" << std::endl;
    std::cout << "  - 'new random beat'" << std::endl;
    std::cout << "  - 'kick on every 4'" << std::endl;
    std::cout << "------------------------------------------" << std::endl;

    CommandParser parser(sequencer, projectManager, kitManager);
    AudioEngine engine(sequencer);

    if (!engine.start()) {
        std::cerr << "Could not start audio engine." << std::endl;
        return 1;
    }

    bool running = true;
    std::thread displayThread([&]() {
        while (running) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });

    std::string line;
    std::cout << "> " << std::flush;
    while (std::getline(std::cin, line)) {
        if (line == "exit" || line == "quit") {
            running = false;
            break;
        }
        parser.parse(line);
        std::cout << "> " << std::flush;
    }

    running = false;
    if (displayThread.joinable()) {
        displayThread.join();
    }

    return 0;
}
