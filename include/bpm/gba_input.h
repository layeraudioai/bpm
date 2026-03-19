#pragma once

#include <string>

namespace bpm
{
    class GBAInput
    {
    public:
        void Init();
        void Update();
        bool IsDown(int key);
        bool IsUp(int key);
        bool IsPressed(int key);
    private:
        int old_keys;
        int new_keys;
    };

    std::string handle_gba_input(GBAInput& gba_input, bool& show_help);
} // namespace bpm
