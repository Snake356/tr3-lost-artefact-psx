// Builds the Lost Artefact PS1 disc image from the player's own games: a Tomb Raider III PS1 disc image (the
// language of the disc to build) and the Lost Artefact PC files (PC disc image, Gold CD or Remastered folder).
//
// Steps: disc files from the image, input levels, sounds, code modules, level conversion, executable (the IPS
// patches named in bin/patch.sh, then the options), texts, pictures (the project's own, the unchanged originals
// from the player's disc), licence sectors, then the disc image from prj/disc/<layout>.xml. The cover (PAL, USA
// or Japan by the disc's region) goes next to the image with the same name.
#pragma once
#include <functional>
#include <string>

#include "../core/files.h"

struct BuildOptions {
    fs::path ps1_image, pc_source, out_bin;
    fs::path ntsc_image;          // empty: none. A European disc as NTSC needs the US disc (its executable)
    std::string blood;            // "", "red", "purple"
    bool level_select = false;    // level select on New Game (and cheats; the Japanese/Asian Disc 1: crystals)
    bool japan_weapons = false;   // Lara's weapons as strong as on the Japanese Disc 1
    bool japan_crystals = false;  // save crystals as on the Japanese Disc 1
    bool keep_work = false;       // keep the work folder (about 1 GB, next to the image by default)
    fs::path work;
};

// log gets the step lines ("disc files", "converting the levels", ...) the window follows. Returns the image.
fs::path build_disc(const BuildOptions &o, const std::function<void(const std::string &)> &log);

// The region cover for a version (pal.png, usa.png, japan.png).
std::string cover_name(const std::string &version);
// The layout (prj/disc/<layout>.xml) of a version.
std::string layout_of(const std::string &version);
// Versions a European disc can be built from as NTSC.
bool ntsc_possible(const std::string &version);
