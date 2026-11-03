// Recognising the player's own game files (own/known_inputs.json).
//   PS1: a Tomb Raider III disc image, recognised by the CRC32 of its game executable (SLxS_xxx.xx, or
//        TOMB3.EXE on the US Rev 1/2 discs, told apart by a marker file).
//   PC:  the Lost Artefact levels and MAIN.SFX from the PC disc or an installed game folder (Gold CD,
//        Tomb Raider I-III Remastered: 3/DATA/LA and 3/SFX).
#pragma once
#include <map>
#include <optional>

#include "disc.h"

extern const char *const PC_LEVELS[6];    // scotland, willsden, chunnel, undersea, zoo, slinc

struct Ps1Info {
    std::string redump, version, language, revision, exe;
    bool supported = false;
};

std::optional<Ps1Info> recognise_ps1(const Disc &disc);

using PcFiles = std::map<std::string, Bytes>;   // "scotland.tr2", ..., "main.sfx"
PcFiles pc_files_from_disc(const Disc &disc);
PcFiles pc_files_from_folder(const fs::path &folder);

struct PcRecognised {
    std::string release;     // empty: unknown version
    bool present = false;
    uint32_t crc = 0;
};
std::map<std::string, PcRecognised> recognise_pc(const PcFiles &files);   // each level and "main.sfx"
