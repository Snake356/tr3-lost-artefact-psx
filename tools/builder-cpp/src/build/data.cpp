#include "data.h"

#include <algorithm>
#include <map>
#include <mutex>

#include "../core/process.h"

namespace {

struct Pack {
    std::map<std::string, Bytes> files;
    std::map<std::string, Json> json;
    std::mutex json_mutex;

    Pack() {
        fs::path path = data_pack_path();
        if (path.empty())
            throw Error("data.pak is missing: it belongs next to the builder (" + path_str(exe_folder() / "data.pak") +
                        " or in its data folder). Unpack the whole ZIP, not only the EXE.");
        Bytes pak = read_file(path);
        size_t n = pak.size();
        if (n < 12 || std::string(pak.begin(), pak.begin() + 8) != "TR3LAPAK")
            throw Error(path_str(path) + " is damaged (not a builder data pack); unpack the ZIP again.");
        uint32_t count = u32(pak, 8);
        size_t o = 12;
        for (uint32_t i = 0; i < count; i++) {
            size_t len = u16(pak, o);
            std::string name(reinterpret_cast<const char *>(&pak[o + 2]), len);
            files[name] = slice(pak, u32(pak, o + 2 + len), u32(pak, o + 6 + len));
            o += 2 + len + 8;
        }
    }
};

Pack &pack() {
    static Pack p;
    return p;
}

}

bool data_has(const std::string &name) { return pack().files.count(name) != 0; }

const Bytes &data_file(const std::string &name) {
    auto it = pack().files.find(name);
    if (it == pack().files.end()) throw Error("the builder's data has no " + name);
    return it->second;
}

std::string data_text(const std::string &name) {
    const Bytes &b = data_file(name);
    std::string s(b.begin(), b.end());
    s.erase(std::remove(s.begin(), s.end(), '\r'), s.end());
    return s;
}

const Json &data_json(const std::string &name) {
    Pack &p = pack();
    std::lock_guard<std::mutex> lock(p.json_mutex);
    auto it = p.json.find(name);
    if (it == p.json.end()) it = p.json.emplace(name, Json::parse(data_text(name))).first;
    return it->second;
}

std::vector<std::string> data_names(const std::string &prefix) {
    std::vector<std::string> out;
    for (auto &kv : pack().files)
        if (starts_with(kv.first, prefix)) out.push_back(kv.first);
    return out;
}

fs::path data_pack_path() {
    for (fs::path p : {exe_folder() / "data.pak", exe_folder() / "data" / "data.pak"})
        if (fs::is_regular_file(p)) return p;
    return {};
}

std::string data_problem() {
    try {
        pack();
        return {};
    } catch (const std::exception &e) {
        return e.what();
    }
}
