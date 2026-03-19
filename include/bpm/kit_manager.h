#pragma once

#include <string>
#include <vector>
#include <memory>
#include "bpm/kit.h"

namespace bpm
{
    class Sequencer; // Forward declaration

    class KitManager
    {
    public:
        KitManager(const std::string& kits_path = "kits");

        bool saveKit(const std::string& name, const Kit& kit);
        bool loadKit(const std::string& name, Sequencer& sequencer);
        std::vector<std::string> listKits();

    private:
        std::string _kits_path;
    };

} // namespace bpm
