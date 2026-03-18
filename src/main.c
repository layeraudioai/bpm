#include <memory>
#include <string>
#include <vector>
#include "bpm/sequencer.h"
#include "bpm/audio_engine.h"
#include "bpm/parser.h"
#include "bpm/project_manager.h"
#include "bpm/kit_manager.h"

#include "bpm/gba_input.h"
#include "bpm/gba_display.h"
#include <gba_input.h>
#include <gba_systemcalls.h>

using namespace bpm;

int main(int argc, char** argv) {
    auto sequencer = std::make_shared<Sequencer>();
    auto projectManager = std::make_shared<ProjectManager>();
    auto kitManager = std::make_shared<KitManager>();
    
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

    CommandParser parser(sequencer, projectManager, kitManager);
    AudioEngine engine(sequencer);

    if (!engine.start()) {
        gba_println("Could not start audio engine.");
        return 1;
    }

    bool running = true;

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

    return 0;
}
