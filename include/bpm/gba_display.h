#pragma once

#include <string>
#include <vector>

namespace bpm
{
    void init_gba_display();
    void gba_print(const std::string& str);
    void gba_println(const std::string& str);
    void gba_clear_screen();
    void gba_printf(const char* fmt, ...);
    void gba_menu_draw(const std::vector<std::string>& commands, int selected_index);
} // namespace bpm
