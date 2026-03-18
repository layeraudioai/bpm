#ifdef __GBA__

#include "bpm/gba_input.h"
#include <gba_input.h>

namespace bpm
{

    void GBAInput::Init()
    {
        scanKeys();
        old_keys = keysHeld();
        new_keys = old_keys;
    }

    void GBAInput::Update()
    {
        old_keys = new_keys;
        scanKeys();
        new_keys = keysHeld();
    }

    bool GBAInput::IsDown(int key)
    {
        return (new_keys & key);
    }

    bool GBAInput::IsUp(int key)
    {
        return !(new_keys & key);
    }

    bool GBAInput::IsPressed(int key)
    {
        return (new_keys & key) && !(old_keys & key);
    }

    std::string handle_gba_input(GBAInput& gba_input, bool& show_help)
    {
        // Placeholder for real input handling
        return "";
    }

} // namespace bpm

#endif // __GBA__
