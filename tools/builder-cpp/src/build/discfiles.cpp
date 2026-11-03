#include "discfiles.h"

#include <map>
#include <set>

#include "../leveltools/tool.h"

DiscLayout disc_layout(const std::string &version) {
    static const std::map<std::string, DiscLayout> layouts = {
        {"us", {"en", "us"}},   {"ukrev", {"en", "uk"}}, {"derev", {"de", "uk"}}, {"frrev", {"fr", "uk"}},
        {"itrev", {"it", "uk"}}, {"sprev", {"sp", "uk"}}, {"jp", {"jp", "jp"}},    {"jp2", {"en", "jp2"}},
        {"as", {"jp", "as"}},   {"as2", {"en", "as2"}},
        // the first releases (Rev 0): the same layouts
        {"us10", {"en", "us"}}, {"uk", {"en", "uk"}},    {"de", {"de", "uk"}},    {"fr", {"fr", "uk"}},
        {"it", {"it", "uk"}},   {"sp", {"sp", "uk"}}};
    auto it = layouts.find(version);
    if (it == layouts.end()) throw Error("no disc layout for version " + version);
    return it->second;
}

namespace {

const size_t XA_SECTOR = 2336;

// interleave slots (sector index % 8) of the XA tracks the Lost Artefact does not use
const std::map<int, std::set<size_t>> XA_SILENT = {{1, {5, 6}}, {2, {7}}, {4, {2, 3, 5, 7}}, {5, {1, 3, 4, 7}},
                                                   {6, {5}}, {8, {4, 5, 6, 7}}};

void silence(Bytes &xa, const std::set<size_t> &slots) {
    for (size_t i = 0; i < xa.size() / XA_SECTOR; i++)
        if (slots.count(i % 8)) std::fill(xa.begin() + long(i * XA_SECTOR), xa.begin() + long((i + 1) * XA_SECTOR), uint8_t(0));
}

}

void make_disc_files(const Disc &disc, const std::string &version, const fs::path &out, const fs::path &work) {
    DiscLayout lay = disc_layout(version);
    for (int n : {1, 2, 3, 4, 5, 6, 7, 8, 10, 16}) {
        Bytes xa = disc.raw_file("XA/F" + std::to_string(n) + ".XA");
        auto s = XA_SILENT.find(n);
        if (s != XA_SILENT.end()) silence(xa, s->second);
        write_file(out / "files" / "xa" / "common" / ("f" + std::to_string(n) + ".xa"), xa);
    }
    for (int n : {11, 12, 13, 14, 15})
        write_file(out / "files" / "xa" / u8path(lay.speech) / ("f" + std::to_string(n) + ".xa"),
                   disc.raw_file("XA/F" + std::to_string(n) + ".XA"));
    fs::path data = out / "files" / "data" / u8path(lay.data);
    write_file(data / "fmv.mod", disc.file("DATA/FMV.MOD"));
    write_file(data / "title.psx", disc.file("DATA/TITLE.PSX"));

    // Lara's Home: the sky dome tip closed, the crawl flip and crouch turns added, Lara's nose textured, two quiet
    // sounds louder and Lara's animation sounds as TRX has them
    fs::path house = work / "house";
    fs::create_directories(house);
    write_file(house / "house_orig.psx", disc.file("DATA/HOUSE.PSX"));
    run_level_tool("close_dome_tip.py", {"house_orig.psx", "house.psx"}, house);
    for (const char *tool : {"psx_lara_anims.py", "house_nose_fix.py", "sound_volumes.py", "lara_anim_sounds.py"})
        run_level_tool(tool, {"house.psx"}, house);
    write_file(data / "house.psx", read_file(house / "house.psx"));

    write_file(out / "files" / "fmv" / "common" / "logo.fmv", disc.raw_file("FMV/LOGO.FMV"));
}
