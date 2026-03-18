#ifdef __GBA__

#include "bpm/gba_display.h"
#include <gba_console.h>
#include <gba_video.h>
#include <gba_systemcalls.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

namespace bpm
{
    void init_gba_display()
    {
        // Initialize the console system
        consoleDemoInit();
    }

    void gba_print(const std::string& str)
    {
        iprintf(str.c_str());
    }

    void gba_println(const std::string& str)
    {
        iprintf((str + "\n  ").c_str());
    }

    void gba_clear_screen()
    {
        // Use ANSI escape code to clear console
        iprintf("\x1b[2J");
    }

    void gba_printf(const char* fmt, ...)
    {
        char buffer[256];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buffer, sizeof(buffer), fmt, args);
        va_end(args);
        iprintf(buffer);
    }
    
    void gba_menu_draw(const std::vector<std::string>& commands, int selected_index)
    {
        gba_clear_screen();
        // iprintf("\x1b[0;0H"); // Move cursor to top-left
        gba_println("BPM for GBA");
        gba_println("Select a command:");
        for (size_t i = 0; i < commands.size(); ++i)
        {
            if ((int)i == selected_index)
            {
                gba_printf("> %s", commands[i].c_str());
            }
            else
            {
                gba_printf("  %s", commands[i].c_str());
            }
        }
    }

} // namespace bpm

#endif // __GBA__
