#include "files.h"

#include <algorithm>
#include <fstream>

Bytes read_file(const fs::path &p) {
    std::ifstream f(p, std::ios::binary);
    if (!f) throw Error("cannot read " + path_str(p));
    f.seekg(0, std::ios::end);
    Bytes b(size_t(f.tellg()));
    f.seekg(0);
    if (!b.empty()) f.read(reinterpret_cast<char *>(b.data()), std::streamsize(b.size()));
    if (!f) throw Error("cannot read " + path_str(p));
    return b;
}

void write_file(const fs::path &p, const Bytes &b) {
    if (p.has_parent_path()) fs::create_directories(p.parent_path());
    std::ofstream f(p, std::ios::binary | std::ios::trunc);
    if (!f) throw Error("cannot write " + path_str(p));
    if (!b.empty()) f.write(reinterpret_cast<const char *>(b.data()), std::streamsize(b.size()));
    if (!f) throw Error("cannot write " + path_str(p));
}

std::string read_text(const fs::path &p) {
    Bytes b = read_file(p);
    std::string s(b.begin(), b.end());
    if (s.size() >= 3 && s.compare(0, 3, "\xEF\xBB\xBF") == 0) s.erase(0, 3);
    s.erase(std::remove(s.begin(), s.end(), '\r'), s.end());
    return s;
}

fs::path u8path(const std::string &s) {
    return fs::path(std::u8string(reinterpret_cast<const char8_t *>(s.data()), s.size()));
}

std::string path_str(const fs::path &p) {
    std::u8string u = p.u8string();
    return std::string(reinterpret_cast<const char *>(u.data()), u.size());
}

std::string lower(std::string s) {
    for (auto &c : s) if (c >= 'A' && c <= 'Z') c = char(c - 'A' + 'a');
    return s;
}

std::string upper(std::string s) {
    for (auto &c : s) if (c >= 'a' && c <= 'z') c = char(c - 'a' + 'A');
    return s;
}

bool ends_with(const std::string &s, const std::string &tail) {
    return s.size() >= tail.size() && s.compare(s.size() - tail.size(), tail.size(), tail) == 0;
}

bool starts_with(const std::string &s, const std::string &head) {
    return s.size() >= head.size() && s.compare(0, head.size(), head) == 0;
}

std::vector<std::string> split(const std::string &s, char sep) {
    std::vector<std::string> out;
    size_t a = 0;
    for (;;) {
        size_t b = s.find(sep, a);
        out.push_back(s.substr(a, b == std::string::npos ? std::string::npos : b - a));
        if (b == std::string::npos) return out;
        a = b + 1;
    }
}

std::string trim(const std::string &s) {
    size_t a = s.find_first_not_of(" \t\n\r"), b = s.find_last_not_of(" \t\n\r");
    return a == std::string::npos ? std::string() : s.substr(a, b - a + 1);
}
