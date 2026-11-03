#include "inputs.h"

#include <algorithm>
#include <set>

#include "data.h"

const char *const PC_LEVELS[6] = {"scotland", "willsden", "chunnel", "undersea", "zoo", "slinc"};

namespace {
bool is_level_name(const std::string &base) {
    for (auto *l : PC_LEVELS)
        if (base == std::string(l) + ".tr2") return true;
    return false;
}
}

std::optional<Ps1Info> recognise_ps1(const Disc &disc) {
    const Json &known = data_json("own/known_inputs.json")["ps1_images"];
    std::vector<std::string> exes{"TOMB3.EXE"};
    for (auto &[path, e] : disc.files()) {
        std::string head = path.substr(0, 4);
        if (path.find('/') == std::string::npos && (head == "SLUS" || head == "SLES" || head == "SLPM" || head == "SCPS"))
            exes.push_back(path);
    }
    for (auto &exe : exes) {
        if (!disc.has(exe)) continue;
        std::string crc = hex8(crc32(disc.file(exe)));
        const Json *hit = nullptr, *marked = nullptr;
        for (auto &i : known.items()) {
            if (i["exe"].str() != exe || i["exe_crc32"].str() != crc) continue;
            if (!hit) hit = &i;
            // discs with the same executable (US Rev 1/2) are told apart by another file
            if (!marked && i.has("marker") && disc.has(i["marker"]["file"].str()) &&
                hex8(crc32(disc.file(i["marker"]["file"].str()))) == i["marker"]["crc32"].str())
                marked = &i;
        }
        if (const Json *i = marked ? marked : hit) {
            Ps1Info info;
            info.redump = (*i)["redump"].str();
            info.version = (*i)["version"].str();
            info.language = (*i)["language"].str();
            info.revision = (*i)["revision"].str();
            info.exe = (*i)["exe"].str();
            info.supported = (*i)["supported"].boolean();
            return info;
        }
    }
    return std::nullopt;
}

PcFiles pc_files_from_disc(const Disc &disc) {
    PcFiles out;
    for (auto &[path, e] : disc.files()) {
        std::string base = lower(path.substr(path.rfind('/') + 1));
        if (starts_with(path, "DATA/") && (is_level_name(base) || base == "main.sfx")) out[base] = disc.file(path);
    }
    return out;
}

PcFiles pc_files_from_folder(const fs::path &folder_in) {
    fs::path folder = fs::absolute(folder_in), up = folder.parent_path();
    // the game folder or its level folder: data (Gold CD install), 3/DATA/LA and 3/SFX (Remastered), the folder
    // itself; the folder above too, for MAIN.SFX next to the level folder
    const fs::path candidates[] = {folder, folder / "data", folder / "DATA", folder / "3" / "DATA" / "LA",
                                   folder / "DATA" / "LA", folder / "3" / "SFX", folder / "SFX", up, up / "SFX",
                                   up.parent_path() / "SFX"};
    PcFiles out;
    for (auto &c : candidates) {
        std::error_code ec;
        if (!fs::is_directory(c, ec)) continue;
        std::vector<fs::path> names;
        for (auto &e : fs::directory_iterator(c, ec)) names.push_back(e.path());
        std::sort(names.begin(), names.end());
        for (auto &p : names) {
            std::string low = lower(path_str(p.filename()));
            if ((is_level_name(low) || low == "main.sfx") && !out.count(low) && fs::is_regular_file(p, ec))
                out[low] = read_file(p);
        }
    }
    return out;
}

std::map<std::string, PcRecognised> recognise_pc(const PcFiles &files) {
    const Json &known = data_json("own/known_inputs.json");
    std::map<std::string, PcRecognised> res;
    for (auto *lv : PC_LEVELS) {
        PcRecognised r;
        auto it = files.find(std::string(lv) + ".tr2");
        if (it != files.end()) {
            r.present = true;
            r.crc = crc32(it->second);
            for (auto &rel : known["pc_levels"].keys())
                if (known["pc_levels"][rel][lv].str() == hex8(r.crc)) {
                    r.release = rel;
                    break;
                }
        }
        res[lv] = r;
    }
    PcRecognised s;
    auto it = files.find("main.sfx");
    if (it != files.end()) {
        s.present = true;
        s.crc = crc32(it->second);
        for (auto &rel : known["pc_main_sfx"].keys())
            if (known["pc_main_sfx"][rel].str() == hex8(s.crc)) {
                s.release = rel;
                break;
            }
    }
    res["main.sfx"] = s;
    return res;
}
