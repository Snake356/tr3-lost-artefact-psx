// The level conversion: the project's levels/convert.sh, done without a shell.
//
// The command lines come from the project's scripts in the data (prj/levels/convert.sh: the lines of the level
// customisation, the level list, conversion parameters and removed sound samples; prj/levels/geom/*.sh;
// prj/levels/japan/*.txt), so convert.sh stays the one place that says how the levels are converted. Its loops
// are done here in the same order, each level in its own folder and the levels side by side.
// The project's edited textures, animations and meshes come as differences to what the converter reads from the
// player's level at that point (own/assets/<level>.tex/.anim/.mesh), so no Core data is shipped.
#pragma once
#include <functional>
#include <string>
#include <vector>

#include "../core/files.h"

using Log = std::function<void(const std::string &)>;

struct ConvertJob {
    fs::path work;            // <work>/<level>/ per level, results in <work>/out/<version>/<level>.psx
    fs::path inputs;          // the seven input levels
    fs::path samples;         // 000.wav..370.wav
    fs::path modules;         // the code module file (tr3pcpsx TR3PCPSX_MODULES)
    std::string version;      // the game version converted for
    Log log;
};

void convert_levels(const ConvertJob &job);
