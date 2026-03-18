#ifdef __GBA__

#include "bpm/gba_display.h"
#include <gba_video.h>
#include <gba_systemcalls.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

namespace bpm
{

    // Basic font for rendering text
    extern const u8 msx_font[];

    // Simple console implementation
    char con_buffer[32 * 32];

    void con_write(const char* ptr)
    {
        strncat(con_buffer, ptr, sizeof(con_buffer) - strlen(con_buffer) - 1);
    }

    void con_clear()
    {
        memset(con_buffer, 0, sizeof(con_buffer));
        REG_BG0VOFS = 0;
    }

    void init_gba_display()
    {
        // Using basic console for now
        consoleDemoInit();
    }

    void gba_print(const std::string& str)
    {
        iprintf(str.c_str());
    }

    void gba_println(const std::string& str)
    {
        iprintf((str + "
").c_str());
    }

    void gba_clear_screen()
    {
        // iprintf("\x1b[2J"); //not working
        con_clear();
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
        gba_println("BPM for GBA");
        gba_println("Select a command:");
        for (int i = 0; i < commands.size(); ++i)
        {
            if (i == selected_index)
            {
                gba_printf("> %s
", commands[i].c_str());
            }
            else
            {
                gba_printf("  %s
", commands[i].c_c_str());
            }
        }
    }


} // namespace bpm

#endif // __GBA__
