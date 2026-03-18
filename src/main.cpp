#include <iostream>
#include <memory>
#include <thread>
#include <chrono>
#include <string>
#include <vector>
#include <ctime>
#include "bpm/sequencer.h"
#include "bpm/audio_engine.h"
#include "bpm/parser.h"
#include "bpm/project_manager.h"
#include "bpm/kit_manager.h"

#ifdef __GBA__
#include "bpm/gba_input.h"
#include "bpm/gba_display.h"
#include <gba_input.h>
#include <gba_systemcalls.h>
#endif

using namespace bpm;

void printHelp() {
#ifndef __GBA__
    std::cout << "Usage: bpm [options]" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -n            Start a new project" << std::endl;
    std::cout << "  -l <name>     Load a project on boot" << std::endl;
    std::cout << "  -h            Show this help" << std::endl;
#endif
}

int main(int argc, char** argv) {
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    auto sequencer = std::make_shared<Sequencer>();
    auto projectManager = std::make_shared<ProjectManager>();
    auto kitManager = std::make_shared<KitManager>();
    
#ifdef __GBA__
    init_gba_display();
    GBAInput gba_input;
    gba_input.Init();

    std::vector<std::string> commands = {
        "new random beat",
        "set song_mode on",
        "set song_mode off",
        "new random kit",
        "add random style",
        "set steps 16",
        "set steps 32",
        "exit"
    };
    int selected_command = 0;
    bool needs_redraw = true;

#else
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
#endif

    CommandParser parser(sequencer, projectManager, kitManager);
    AudioEngine engine(sequencer);

    if (!engine.start()) {
#ifdef __GBA__
        gba_println("Could not start audio engine.");
#else
        std::cerr << "Could not start audio engine." << std::endl;
#endif
        return 1;
    }

    bool running = true;

#ifdef __GBA__
    while (running) {
        VBlankIntrWait();
        gba_input.Update();

        if (gba_input.IsPressed(KEY_DOWN)) {
            selected_command = (selected_command + 1) % commands.size();
            needs_redraw = true;
        }
        if (gba_input.IsPressed(KEY_UP)) {
            selected_command = (selected_command - 1 + commands.size()) % commands.size();
            needs_redraw = true;
        }
        if (gba_input.IsPressed(KEY_A)) {
            const std::string& cmd = commands[selected_command];
            if (cmd == "exit") {
                running = false;
            } else {
                // Clear the screen to show command output (if any)
                // then redraw the menu on next B press
                gba_clear_screen();
                parser.parse(cmd);
                gba_println("\nPress B to return to menu");
            }
        }
        
        if (gba_input.IsPressed(KEY_B)) {
            // go back to menu
            needs_redraw = true;
        }

        if (needs_redraw) {
            gba_menu_draw(commands, selected_command);
            needs_redraw = false;
        }
    }
#else
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
#endif

    return 0;
}
