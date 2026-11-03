// What the build takes from the player's files besides the levels' conversion: the converter's input levels,
// the sound samples, the code modules, the game texts, the Kanji font and the licence sectors.
#pragma once
#include "inputs.h"

// The seven input levels (scotland.tr2 ... slinc.tr2, underse2.tr2) from the player's PC levels: each as it is
// or turned into the project's input by its patch (own/levels.json); underse2 starts as a copy of undersea.
std::map<std::string, Bytes> input_levels(const PcFiles &pc);

// The 371 samples (000.wav..370.wav) from MAIN.SFX (Gold CD: the WAV files in a row; Remastered: a table, then
// 414 WAV files); each checked against its CRC32 (own/sounds.json).
std::vector<Bytes> sound_samples(const Bytes &main_sfx);

// The code module file for the converter (tr3pcpsx -DEXTERNAL_MODULES): the modules cut out of the disc's own
// level files with the project's byte changes (own/modules.json).
Bytes module_file(const Disc &disc, const std::string &version);

// TOMBPSX.DAT (gameflow and strings) of a layout; level_select: level select on New Game (the Japanese and
// Asian Disc 1 take their own gameflow with a save crystal at each level start, the others get the flags).
Bytes game_texts(const std::string &layout, const Disc &disc, bool level_select);
Bytes kanji_font(const Disc &disc);
bool is_japanese_layout(const std::string &layout);

// The licence sectors (0-11) in the form the disc writer takes: subheader, 2048 data bytes, empty EDC/ECC.
Bytes licence_sectors(const Disc &disc);
