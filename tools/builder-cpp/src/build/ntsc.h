// A European disc as NTSC (60 Hz): the US executable with the language's speech and texts.
#pragma once
#include <string>

#include "../core/bytes.h"

// The NTSC executable from the patched US executable and the language's patched PAL executable: the language's
// XA clip lengths for F11-F15 (the start comes from the file, only the end offsets differ) and its best-times
// headings in Lara's Home (already in the US executable). lang: en, de, fr, it, sp.
Bytes make_ntsc_exe(const Bytes &us_exe, const Bytes &lang_exe, const std::string &lang);

// TOMBPSX.DAT of a European NTSC disc: the US memory card name with the language's letter instead of the 3
// (BASLUS-00691TRFG for French), so its savegames are kept apart from the US and PAL discs'.
Bytes ntsc_card_name(const Bytes &dat, const std::string &lang);

// The memory card names of a game run by a first release executable (Rev 0): T0 instead of TR (Rev 0 and
// Rev 1 savegames are not compatible). The last letter (G normal, P New Game+) stays.
Bytes rev0_card_names(const Bytes &dat);

// The NTSC layout of a European language's PAL layout: US licence, SYSTEM.CNF and executable name, PIXUS
// pictures, the US data files and levels, and the title picture of 240 lines.
std::string ntsc_layout(std::string xml, const std::string &layout);
