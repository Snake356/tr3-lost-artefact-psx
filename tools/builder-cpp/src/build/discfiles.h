// The files the Lost Artefact disc takes over from the player's Tomb Raider III disc, in the folder structure
// the disc layouts (prj/disc/<layout>.xml) name:
//   files/xa/common/f1..f8, f10, f16.xa   speech/music streams as raw 2336-byte sectors; in F1, F2, F4, F5, F6
//                                         and F8 the interleave slots of tracks the Lost Artefact does not use
//                                         are zeroed
//   files/xa/<language>/f11..f15.xa       the language's own speech
//   files/data/<data>/fmv.mod, title.psx  as on the disc
//   files/data/<data>/house.psx           Lara's Home with the project's changes (level tools)
//   files/fmv/common/logo.fmv             the Eidos logo video
#pragma once
#include "inputs.h"

struct DiscLayout {
    std::string speech, data;     // folder of F11-F15, data folder
};
DiscLayout disc_layout(const std::string &version);

// Writes the disc files of the disc into out (house.psx is made in work/house).
void make_disc_files(const Disc &disc, const std::string &version, const fs::path &out, const fs::path &work);
