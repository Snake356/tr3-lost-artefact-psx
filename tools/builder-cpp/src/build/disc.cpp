#include "disc.h"

#include <algorithm>

#include "../core/inflate.h"

Disc::Disc(const fs::path &image) {
    f_ = _wfopen(image.wstring().c_str(), L"rb");
    if (!f_) throw Error("cannot read " + path_str(image));
    const int kinds[3][2] = {{2352, 24}, {2352, 16}, {2048, 0}};
    for (auto &k : kinds) {
        char id[5];
        _fseeki64(f_, int64_t(16) * k[0] + k[1] + 1, SEEK_SET);
        if (fread(id, 1, 5, f_) == 5 && std::string(id, 5) == "CD001") {
            ss_ = k[0];
            off_ = k[1];
            break;
        }
    }
    if (!ss_) throw Error(path_str(image.filename()) + ": no ISO 9660 file system found");
    if (ss_ == 2352) {                               // mode 1 or mode 2: the data follows the mode byte
        uint8_t mode = 0;
        _fseeki64(f_, int64_t(16) * 2352 + 15, SEEK_SET);
        if (fread(&mode, 1, 1, f_) != 1) throw Error("cannot read the disc image");
        off_ = mode == 1 ? 16 : 24;
    }
    Bytes pvd = read(16, 2048);
    walk(u32(pvd, 158), u32(pvd, 166), "", 0);
}

Disc::~Disc() {
    if (f_) fclose(f_);
}

Bytes Disc::read(uint32_t lba, uint32_t size) const {
    uint32_t n = (size + 2047) / 2048;
    Bytes out;
    if (!n) return out;
    Bytes raw(size_t(n - 1) * size_t(ss_) + 2048);
    _fseeki64(f_, int64_t(lba) * ss_ + off_, SEEK_SET);
    size_t got = fread(raw.data(), 1, raw.size(), f_);
    raw.resize(got);
    if (ss_ == 2048) {
        raw.resize(std::min<size_t>(raw.size(), size));
        return raw;
    }
    out.reserve(size_t(n) * 2048);
    for (uint32_t i = 0; i < n && size_t(i) * ss_ < raw.size(); i++) {
        size_t a = size_t(i) * ss_, b = std::min(raw.size(), a + 2048);
        out.insert(out.end(), raw.begin() + long(a), raw.begin() + long(b));
    }
    out.resize(std::min<size_t>(out.size(), size));
    return out;
}

void Disc::walk(uint32_t lba, uint32_t size, const std::string &prefix, int depth) {
    if (depth > 16) throw Error("the disc's directories nest too deep");
    Bytes d = read(lba, size);
    for (size_t p = 0; p < d.size();) {
        uint8_t len = d[p];
        if (len == 0) {
            p = (p / 2048 + 1) * 2048;
            continue;
        }
        if (p + 33 > d.size()) break;
        uint32_t flb = u32(d, p + 2), fsz = u32(d, p + 10);
        uint8_t flags = d[p + 25], nl = d[p + 32];
        std::string name(reinterpret_cast<const char *>(&d[std::min(d.size(), p + 33)]),
                         std::min<size_t>(nl, d.size() - std::min(d.size(), p + 33)));
        p += len;
        if (name == std::string(1, '\0') || name == std::string(1, '\1')) continue;
        name = upper(name.substr(0, name.find(';')));
        if (flags & 2) walk(flb, fsz, prefix + name + "/", depth + 1);
        else files_[prefix + name] = {flb, fsz};
    }
}

Bytes Disc::file(const std::string &path) const {
    auto it = files_.find(upper(path));
    if (it == files_.end()) throw Error(path + " is not on the disc");
    return read(it->second.lba, it->second.size);
}

Bytes Disc::sectors(uint32_t lba, uint32_t count, uint32_t offset, uint32_t length) const {
    Bytes out;
    Bytes s(static_cast<size_t>(ss_));
    for (uint32_t i = 0; i < count; i++) {
        _fseeki64(f_, int64_t(lba + i) * ss_, SEEK_SET);
        if (fread(s.data(), 1, s.size(), f_) != s.size()) throw Error("the disc image ends too early");
        out.insert(out.end(), s.begin() + offset, s.begin() + offset + length);
    }
    return out;
}

Bytes Disc::raw_file(const std::string &path) const {
    if (ss_ != 2352) throw Error(path + ": streamed files need a raw (2352-byte sector) image, not an .iso");
    auto it = files_.find(upper(path));
    if (it == files_.end()) throw Error(path + " is not on the disc");
    uint32_t n = (it->second.size + 2047) / 2048;
    if (!n) return {};
    Bytes raw(size_t(n - 1) * 2352 + 2336);
    _fseeki64(f_, int64_t(it->second.lba) * 2352 + 16, SEEK_SET);
    raw.resize(fread(raw.data(), 1, raw.size(), f_));
    Bytes out;
    out.reserve(size_t(n) * 2336);
    for (uint32_t i = 0; i < n && size_t(i) * 2352 < raw.size(); i++) {
        size_t a = size_t(i) * 2352, b = std::min(raw.size(), a + 2336);
        out.insert(out.end(), raw.begin() + long(a), raw.begin() + long(b));
    }
    return out;
}

std::unique_ptr<Disc> open_image(const fs::path &path, const fs::path &unpack_dir, std::string *how) {
    std::string low = lower(path_str(path.extension()));
    std::string name = path_str(path.filename());
    if (low == ".zip") {
        ZipArchive z(path);
        const ZipEntry *best = nullptr;
        for (auto &e : z.entries()) {
            std::string n = lower(e.name);
            if ((ends_with(n, ".bin") || ends_with(n, ".iso") || ends_with(n, ".img")) && (!best || e.size > best->size))
                best = &e;                              // the data track is the big one
        }
        if (!best) throw Error(name + ": the zip holds no .bin/.iso image");
        fs::create_directories(unpack_dir);
        size_t k = 0;
        while (fs::exists(unpack_dir / (std::to_string(k) + ".bin"))) k++;
        fs::path dst = unpack_dir / (std::to_string(k) + ".bin");
        z.extract(*best, dst);
        if (how) *how = name + " -> " + best->name;
        return std::make_unique<Disc>(dst);
    }
    if (low == ".cue") {
        for (auto &line : split(read_text(path), '\n')) {
            std::string t = trim(line);
            if (upper(t.substr(0, 4)) != "FILE") continue;
            std::string file;
            size_t q = t.find('"');
            if (q != std::string::npos) file = t.substr(q + 1, t.find('"', q + 1) - q - 1);
            else file = split(t, ' ').size() > 1 ? split(t, ' ')[1] : "";
            if (how) *how = name + " -> " + file;
            return std::make_unique<Disc>(path.parent_path() / u8path(file));
        }
        throw Error(name + ": the cue sheet names no file");
    }
    if (how) *how = name;
    return std::make_unique<Disc>(path);
}
