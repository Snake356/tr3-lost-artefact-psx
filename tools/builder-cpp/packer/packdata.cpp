// Collects what the builder needs at run time into one data pack (data.pak, shipped next to the builder EXE):
//   own/...    the builder's data from <project>/tools/builder (patches, assets, tables, covers, icon)
//   prj/...    project files: executable patches, conversion scripts, disc layouts and the pictures they use
// The conversion programs (tr3pcpsx, trmod, ...) are not in it: they are files in tools\ next to the EXE.
// No game data: the pictures the layouts take from the player's own disc are left out.
//
// Usage: packdata <project folder> <out.pak>   (the pack is only rewritten when it changed)
#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <regex>
#include <set>
#include <string>
#include <vector>

namespace fs = std::filesystem;

static std::string read_all(const fs::path &p) {
    std::ifstream f(p, std::ios::binary);
    if (!f) throw std::runtime_error("cannot read " + p.string());
    return std::string(std::istreambuf_iterator<char>(f), {});
}

static std::string rel(const fs::path &p, const fs::path &base) { return fs::relative(p, base).generic_string(); }

static void add_tree(std::map<std::string, fs::path> &out, const std::string &prefix, const fs::path &dir) {
    if (!fs::exists(dir)) throw std::runtime_error("missing " + dir.string());
    for (auto &e : fs::recursive_directory_iterator(dir))
        if (e.is_regular_file()) out[prefix + rel(e.path(), dir)] = e.path();
}

static void add_glob(std::map<std::string, fs::path> &out, const fs::path &prj, const std::string &dir,
                     const std::regex &name) {
    for (auto &e : fs::recursive_directory_iterator(prj / dir)) {
        if (!e.is_regular_file()) continue;
        std::string r = rel(e.path(), prj);
        if (std::regex_match(r, name)) out["prj/" + r] = e.path();
    }
}

int main(int argc, char **argv) {
    if (argc != 3) {
        std::cerr << "usage: packdata <project folder> <out.pak>\n";
        return 2;
    }
    try {
        fs::path prj = argv[1], out_path = argv[2];
        fs::path own = prj / "tools" / "builder";
        std::map<std::string, fs::path> files;
        for (const char *d : {"assets", "levels", "texts", "covers"}) add_tree(files, std::string("own/") + d + "/", own / d);
        for (const char *f : {"known_inputs.json", "levels.json", "modules.json", "sounds.json", "texts.json",
                              "pictures_from_disc.json", "icon.ico", "icon.png"})
            files[std::string("own/") + f] = own / f;
        for (const char *f : {"bin/patch.sh", "levels/convert.sh", "levels/sfx/empty.wav", "art/lara/nose.bin"})
            files[std::string("prj/") + f] = prj / f;
        add_glob(files, prj, "bin/patches", std::regex(R"(bin/patches/[^/]+/[^/]+\.ips)"));
        add_glob(files, prj, "levels/geom", std::regex(R"(levels/geom/[^/]+\.sh)"));
        add_glob(files, prj, "levels/japan", std::regex(R"(levels/japan/[^/]+\.txt)"));
        add_glob(files, prj, "disc/files/system", std::regex(R"(disc/files/system/[^/]+\.cnf)"));
        add_glob(files, prj, "art/loadscr", std::regex(R"(art/loadscr/[^/]+/title[^/]*\.raw)"));

        // the layouts and the pictures they name that are the project's own (not taken from the player's disc)
        std::set<std::string> from_disc{"art/kanji/kanji_la.psx"};
        std::string pics = read_all(own / "pictures_from_disc.json");
        std::regex key(R"rx("(art/[^"]+)"\s*:)rx");
        for (std::sregex_iterator it(pics.begin(), pics.end(), key), end; it != end; ++it) from_disc.insert((*it)[1]);
        std::regex src(R"rx(source="\.\./(art/[^"]+)")rx");
        for (const char *layout : {"us", "uk", "de", "fr", "it", "sp", "jp", "jp2", "as", "as2"}) {
            std::string x = std::string("disc/") + layout + ".xml";
            files["prj/" + x] = prj / x;
            std::string xml = read_all(prj / x);
            for (std::sregex_iterator it(xml.begin(), xml.end(), src), end; it != end; ++it) {
                std::string a = (*it)[1];
                if (!from_disc.count(a) && fs::exists(prj / a)) files["prj/" + a] = prj / a;
            }
        }

        // TR3LAPAK, u32 count, then per file: u16 name length, name, u32 offset, u32 size; then the data
        std::string head = "TR3LAPAK", body;
        auto u32 = [](std::string &s, uint32_t v) { for (int i = 0; i < 4; i++) s += char(v >> (8 * i)); };
        u32(head, uint32_t(files.size()));
        size_t index_size = head.size();
        for (auto &[name, p] : files) index_size += 2 + name.size() + 8;
        std::string index;
        for (auto &[name, p] : files) {
            std::string data = read_all(p);
            index += char(name.size() & 0xFF);
            index += char(name.size() >> 8);
            index += name;
            u32(index, uint32_t(index_size + body.size()));
            u32(index, uint32_t(data.size()));
            body += data;
        }
        std::string pak = head + index + body;
        if (fs::exists(out_path) && read_all(out_path) == pak) return 0;
        fs::create_directories(out_path.parent_path());
        std::ofstream(out_path, std::ios::binary).write(pak.data(), std::streamsize(pak.size()));
        std::cout << "data pack: " << files.size() << " files, " << pak.size() / 1024 << " KB\n";
        return 0;
    } catch (const std::exception &e) {
        std::cerr << "packdata: " << e.what() << "\n";
        return 1;
    }
}
