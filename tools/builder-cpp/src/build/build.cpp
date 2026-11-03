#include "build.h"

#include <map>
#include <mutex>
#include <regex>

#include "../core/patches.h"
#include "../core/process.h"
#include "convert.h"
#include "data.h"
#include "discfiles.h"
#include "discwriter.h"
#include "ntsc.h"
#include "sources.h"

namespace {

const std::map<std::string, std::string> LAYOUT = {
    {"us", "us"}, {"ukrev", "uk"}, {"derev", "de"}, {"frrev", "fr"}, {"itrev", "it"}, {"sprev", "sp"},
    {"jp", "jp"}, {"jp2", "jp2"}, {"as", "as"},   {"as2", "as2"},
    {"us10", "us"}, {"uk", "uk"},  {"de", "de"},    {"fr", "fr"},    {"it", "it"},    {"sp", "sp"}};    // Rev 0
// the speech and heading language of a European disc built as NTSC
const std::map<std::string, std::string> NTSC_LANG = {
    {"derev", "de"}, {"frrev", "fr"}, {"itrev", "it"}, {"sprev", "sp"}, {"ukrev", "en"},
    {"de", "de"},    {"fr", "fr"},    {"it", "it"},    {"sp", "sp"},    {"uk", "en"}};
bool japan_built_in(const std::string &v) { return v == "jp" || v == "as"; }     // the Japanese/Asian Disc 1 have it
bool first_release(const std::string &v) { return v == "de" || v == "fr" || v == "it" || v == "sp" || v == "uk" || v == "us10"; }

Bytes ips(const std::string &version, const std::string &name) {
    return data_file("prj/bin/patches/" + version + "/" + name + ".ips");
}

// The executable with the project's patches (bin/patch.sh) and the options. ntsc_lang: the US executable of a
// European NTSC disc gets the savegame title and the New Game+ texts in that language.
Bytes patched_exe(const Disc &disc, const Ps1Info &info, const std::string &blood, bool japan_weapons,
                  const std::string &ntsc_lang, bool japan_crystals) {
    Bytes exe = disc.file(info.exe);
    std::smatch m;
    std::string sh = data_text("prj/bin/patch.sh");
    if (!std::regex_search(sh, m, std::regex("(^|\\n)patches=\"([^\"]*)\""))) throw Error("bin/patch.sh: no patch list");
    for (auto name : split(m[2], ',')) {
        if (name.empty()) continue;
        if ((name == "savename" || name == "ngpchoice") && !ntsc_lang.empty()) name += "_" + ntsc_lang;
        exe = apply_ips(exe, ips(info.version, name));
    }
    if (!blood.empty()) exe = apply_ips(exe, ips(info.version, "blood" + blood));
    if (japan_weapons && !japan_built_in(info.version)) exe = apply_ips(exe, ips(info.version, "japanweapons"));
    if (japan_crystals && !japan_built_in(info.version)) exe = apply_ips(exe, ips(info.version, "japancrystals"));
    return exe;
}

std::string xml_attr_path(const fs::path &p) { return path_str(fs::absolute(p).make_preferred()); }

}

std::string layout_of(const std::string &version) {
    auto it = LAYOUT.find(version);
    if (it == LAYOUT.end()) throw Error("unknown game version " + version);
    return it->second;
}

bool ntsc_possible(const std::string &version) { return NTSC_LANG.count(version) != 0; }

std::string cover_name(const std::string &v) {
    if (v == "us" || v == "us10") return "usa.png";
    if (v == "jp" || v == "jp2" || v == "as" || v == "as2") return "japan.png";
    return "pal.png";             // every European disc, also as NTSC
}

fs::path build_disc(const BuildOptions &o, const std::function<void(const std::string &)> &log_in) {
    std::mutex log_mutex;
    Log log = [&](const std::string &s) {
        std::lock_guard<std::mutex> lock(log_mutex);
        log_in(s);
    };
    fs::path out_bin = fs::absolute(o.out_bin);
    fs::path work = fs::absolute(o.work.empty() ? out_bin.parent_path() / "build_work" : o.work);
    if (fs::exists(work)) fs::remove_all(work);
    fs::create_directories(work);

    std::string how;
    auto disc = open_image(o.ps1_image, work / "images", &how);
    auto info = recognise_ps1(*disc);
    if (!info) throw Error(how + ": not a known Tomb Raider III PS1 disc");
    if (!info->supported) throw Error(info->redump + ": this revision is not supported yet");
    std::string layout = layout_of(info->version);
    const Disc *main_disc = disc.get();
    Ps1Info main_info = *info;
    std::unique_ptr<Disc> us_disc;
    if (!o.ntsc_image.empty()) {
        if (!ntsc_possible(info->version))
            throw Error("NTSC is available for the European discs (German, French, Italian, Spanish, English UK)");
        std::string how2;
        us_disc = open_image(o.ntsc_image, work / "images", &how2);
        auto us_info = recognise_ps1(*us_disc);
        if (!us_info || (us_info->version != "us" && us_info->version != "us10") || !us_info->supported)
            throw Error(how2 + ": needs the US Tomb Raider III disc (the NTSC executable)");
        main_disc = us_disc.get();
        main_info = *us_info;
    }
    log("Tomb Raider III " + info->language + " " + info->revision + " -> layout " + layout + (us_disc ? " (NTSC)" : ""));

    log("disc files");
    make_disc_files(*main_disc, main_info.version, work / "disc", work / "disc_work");
    if (us_disc) {                                         // the language's own speech (F11-F15)
        make_disc_files(*disc, info->version, work / "disc_lang", work / "disc_lang_work");
        std::string speech = disc_layout(info->version).speech;
        fs::path dst = work / "disc" / "files" / "xa" / u8path(speech);
        fs::remove_all(dst);
        fs::copy(work / "disc_lang" / "files" / "xa" / u8path(speech), dst, fs::copy_options::recursive);
    }

    log("Lost Artefact PC files");
    PcFiles pc;
    std::unique_ptr<Disc> pc_disc;
    if (fs::is_directory(o.pc_source)) {
        pc = pc_files_from_folder(o.pc_source);
    } else {
        pc_disc = open_image(o.pc_source, work / "images");
        pc = pc_files_from_disc(*pc_disc);
        pc_disc.reset();
    }
    std::string missing;
    for (auto *lv : PC_LEVELS)
        if (!pc.count(std::string(lv) + ".tr2")) missing += (missing.empty() ? "" : ", ") + std::string(lv) + ".tr2";
    if (!pc.count("main.sfx")) missing += (missing.empty() ? "" : ", ") + std::string("main.sfx (MAIN.SFX holds the sounds)");
    if (!missing.empty()) throw Error(path_str(o.pc_source) + ": missing " + missing);
    for (auto &[name, data] : input_levels(pc)) write_file(work / "in" / u8path(name), data);
    {
        std::vector<Bytes> samples = sound_samples(pc.at("main.sfx"));
        for (size_t i = 0; i < samples.size(); i++) {
            char n[16];
            snprintf(n, sizeof n, "%03zu.wav", i);
            write_file(work / "sfx" / n, samples[i]);
        }
    }
    pc.clear();

    std::string conv_version = main_info.version;
    write_file(work / "modules.bin", module_file(*main_disc, conv_version));
    log("converting the levels");
    convert_levels({work / "conv", work / "in", work / "sfx", work / "modules.bin", conv_version,
                    [&](const std::string &m) { log("  " + m); }});

    log("executable, texts, pictures");
    std::string ntsc_lang = us_disc ? NTSC_LANG.at(info->version) : "";
    Bytes exe = patched_exe(*main_disc, main_info, o.blood, o.japan_weapons, ntsc_lang, o.japan_crystals);
    Bytes dat = game_texts(layout, *disc, o.level_select);
    if (us_disc) {
        exe = make_ntsc_exe(exe, patched_exe(*disc, *info, "", false, "", false), ntsc_lang);
        put8(dat, 383, 3);                                 // LANGUAGE AMERICAN (no German blood censorship)
        dat = ntsc_card_name(dat, ntsc_lang);
    }
    if (first_release(main_info.version)) dat = rev0_card_names(dat);   // the executable decides
    write_file(work / "game.exe", exe);
    write_file(work / "TOMBPSX.DAT", dat);
    if (is_japanese_layout(layout)) write_file(work / "KANJI.PSX", kanji_font(*disc));
    write_file(work / "licence.dat", licence_sectors(*main_disc));

    std::string xml = data_text("prj/disc/" + layout + ".xml");
    if (us_disc) xml = ntsc_layout(xml, layout);
    const Json &pictures = data_json("own/pictures_from_disc.json")["from_player_disc"];
    auto source = [&](const std::string &src) -> fs::path {
        fs::path p;
        if (starts_with(src, "./files/system/")) {
            p = work / "system" / u8path(src.substr(15));
            write_file(p, data_file("prj/disc/" + src.substr(2)));
        } else if (starts_with(src, "./files/")) {
            p = work / "disc" / u8path(src.substr(2));
        } else if (starts_with(src, "./licenses/")) {
            p = work / "licence.dat";
        } else if (starts_with(src, "../bin/mod/")) {
            p = work / "game.exe";
        } else if (starts_with(src, "../tombpsx/out/")) {
            p = work / "TOMBPSX.DAT";
        } else if (src == "../art/kanji/kanji_la.psx") {
            p = work / "KANJI.PSX";
        } else if (starts_with(src, "../levels/out/")) {
            p = work / "conv" / "out" / u8path(conv_version) / u8path(src.substr(src.rfind('/') + 1));
        } else if (starts_with(src, "../") && pictures.has(src.substr(3))) {
            p = work / "pix" / u8path(src.substr(src.rfind('/') + 1));
            write_file(p, disc->file(pictures[src.substr(3)].str()));   // the language's own pictures
        } else if (starts_with(src, "../art/")) {
            p = work / "art" / u8path(src.substr(3));
            if (!fs::exists(p)) write_file(p, data_file("prj/" + src.substr(3)));
        } else {
            throw Error("disc/" + layout + ".xml: unknown source " + src);
        }
        if (!fs::exists(p)) throw Error("missing " + path_str(p));
        return p;
    };
    std::string filled;
    std::regex attr("(source|<license file)=\"([^\"]+)\"");
    size_t last = 0;
    for (std::sregex_iterator it(xml.begin(), xml.end(), attr), end; it != end; ++it) {
        filled += xml.substr(last, size_t(it->position()) - last);
        filled += (*it)[1].str() + "=\"" + xml_attr_path(source((*it)[2])) + "\"";
        last = size_t(it->position() + it->length());
    }
    filled += xml.substr(last);
    std::string name = path_str(out_bin.filename());
    filled = std::regex_replace(filled, std::regex("image_name=\"[^\"]*\" cue_sheet=\"[^\"]*\""),
                                "image_name=\"" + name + "\" cue_sheet=\"" + name.substr(0, name.size() - 4) + ".cue\"");
    write_file(work / "disc.xml", to_bytes(filled));

    log("writing the disc image");
    disc.reset();
    us_disc.reset();
    write_disc_image(filled, out_bin);
    fs::path cover = out_bin;
    cover.replace_extension(".png");                       // found by frontends next to the disc image
    write_file(cover, data_file("own/covers/" + cover_name(info->version)));
    if (!o.keep_work) {
        std::error_code ec;
        fs::remove_all(work, ec);
    }
    log("done: " + path_str(out_bin));
    return out_bin;
}
