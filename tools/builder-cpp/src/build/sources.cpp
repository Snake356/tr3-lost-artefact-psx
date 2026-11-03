#include "sources.h"

#include "../core/patches.h"
#include "data.h"

std::map<std::string, Bytes> input_levels(const PcFiles &pc) {
    const Json &table = data_json("own/levels.json");
    std::map<std::string, Bytes> out;
    for (auto *lv : PC_LEVELS) {
        const Bytes &data = pc.at(std::string(lv) + ".tr2");
        std::string crc = hex8(crc32(data));
        if (!table[lv].has(crc)) throw Error(std::string(lv) + ".tr2: unknown version of the level");
        const std::string &how = table[lv][crc].str();
        out[std::string(lv) + ".tr2"] = how == "same" ? data : apply_bindiff(data, data_file("own/levels/" + how));
    }
    const Json &u2 = table["underse2"];
    if (u2.is_object() && u2.has("from_undersea"))       // only when the project's underse2 input was edited on its own
        out["underse2.tr2"] = apply_bindiff(out["undersea.tr2"], data_file("own/levels/" + u2["from_undersea"].str()));
    else
        out["underse2.tr2"] = out["undersea.tr2"];
    return out;
}

namespace {
std::vector<Bytes> split_waves(const Bytes &data, size_t p) {
    std::vector<Bytes> out;
    while (p < data.size()) {
        if (p + 8 > data.size() || std::string(data.begin() + long(p), data.begin() + long(p + 4)) != "RIFF")
            throw Error("MAIN.SFX: no WAV file at byte " + std::to_string(p));
        size_t n = size_t(u32(data, p + 4)) + 8;
        out.push_back(slice(data, p, std::min(n, data.size() - p)));
        p += n;
    }
    return out;
}
}

std::vector<Bytes> sound_samples(const Bytes &main_sfx) {
    const Json &table = data_json("own/sounds.json");
    std::vector<Bytes> waves;
    if (main_sfx.size() >= 4 && std::string(main_sfx.begin(), main_sfx.begin() + 4) == "RIFF") {
        waves = split_waves(main_sfx, 0);
    } else {
        const Json &rem = table["remastered"];
        std::vector<Bytes> all = split_waves(main_sfx, size_t(rem["header_bytes"].integer()));
        for (auto &i : rem["sample_index"].items()) {
            size_t k = size_t(i.integer());
            if (k >= all.size()) throw Error("MAIN.SFX: fewer samples than expected");
            waves.push_back(all[k]);
        }
    }
    const auto &crcs = table["samples_crc32"].items();
    if (waves.size() != crcs.size())
        throw Error("MAIN.SFX: " + std::to_string(waves.size()) + " samples instead of " + std::to_string(crcs.size()));
    for (size_t i = 0; i < waves.size(); i++)
        if (hex8(crc32(waves[i])) != crcs[i].str())
            throw Error("MAIN.SFX: sample " + std::to_string(i) + " differs from the Lost Artefact sample");
    return waves;
}

Bytes module_file(const Disc &disc, const std::string &version) {
    const Json &table = data_json("own/modules.json")[version];
    std::vector<std::pair<long long, const Json *>> entries;
    for (auto &[k, e] : table.members()) entries.push_back({std::stoll(k), &e});
    std::sort(entries.begin(), entries.end(), [](auto &a, auto &b) { return a.first < b.first; });
    std::map<std::string, Bytes> cache;
    Bytes out;
    Writer w{out};
    w.u32(uint32_t(entries.size()));
    for (auto &[index, e] : entries) {
        const std::string &file = (*e)["file"].str();
        if (!cache.count(file)) cache[file] = disc.file(file);
        const Bytes &src = cache[file];
        size_t off = size_t((*e)["offset"].integer()), len = size_t((*e)["length"].integer());
        Bytes code = slice(src, off, std::min(len, src.size() > off ? src.size() - off : 0));
        if (hex8(crc32(code)) != (*e)["crc32"].str())
            throw Error(file + ": code module " + (*e)["name"].str() + " differs from the expected one");
        if (e->has("change"))
            for (auto &[pos, val] : (*e)["change"].members()) put8(code, size_t(std::stoll(pos)), uint32_t(val.integer()));
        w.u32(uint32_t(index)).u32(uint32_t(code.size())).bytes(code);
    }
    return out;
}

namespace {
const uint16_t TEST_FLAGS = 0x0C80;      // gameflow flags: level select on New Game, cheat codes, DOZY (fly)
bool saves_from_inventory(const std::string &layout) { return layout == "jp" || layout == "as"; }
}

bool is_japanese_layout(const std::string &layout) {
    return layout == "jp" || layout == "jp2" || layout == "as" || layout == "as2";
}

Bytes game_texts(const std::string &layout, const Disc &disc, bool level_select) {
    std::string key = level_select && saves_from_inventory(layout) ? layout + "test" : layout;
    const Json &e = data_json("own/texts.json")[key];
    Bytes dat = apply_bindiff(disc.file(e["base_file"].str()), data_file("own/texts/" + e["patch"].str()));
    if (level_select && !saves_from_inventory(layout)) put16(dat, 374, u16(dat, 374) | TEST_FLAGS);
    return dat;
}

Bytes kanji_font(const Disc &disc) {
    const Json &e = data_json("own/texts.json")["kanji"];
    return apply_bindiff(disc.file(e["base_file"].str()), data_file("own/texts/" + e["patch"].str()));
}

Bytes licence_sectors(const Disc &disc) {
    Bytes out;
    const uint8_t sub[8] = {0, 0, 8, 0, 0, 0, 8, 0};
    for (uint32_t s = 0; s < 12; s++) {
        out.insert(out.end(), sub, sub + 8);
        Bytes d = disc.sectors(s, 1, 24, 2048);
        out.insert(out.end(), d.begin(), d.end());
        out.resize(out.size() + 280);
    }
    return out;
}
