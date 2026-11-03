// Reading the players' disc images: the ISO 9660 file system of a .bin (raw 2352-byte sectors, mode 1 or 2),
// an .iso (2048-byte sectors), a .cue naming its .bin, or a .zip holding one of them.
#pragma once
#include <cstdio>
#include <map>
#include <memory>
#include <string>

#include "../core/files.h"

class Disc {
public:
    explicit Disc(const fs::path &image);
    ~Disc();
    Disc(const Disc &) = delete;
    Disc &operator=(const Disc &) = delete;

    struct Entry {
        uint32_t lba, size;
    };
    const std::map<std::string, Entry> &files() const { return files_; }   // "DATA/HOUSE.PSX" (upper case)
    bool has(const std::string &path) const { return files_.count(upper(path)) != 0; }
    Bytes file(const std::string &path) const;
    Bytes raw_file(const std::string &path) const;      // mode 2 form 2 streams as 2336-byte sectors
    Bytes sectors(uint32_t lba, uint32_t count, uint32_t offset, uint32_t length) const;  // part of raw sectors
    int sector_size() const { return ss_; }

private:
    FILE *f_ = nullptr;
    int ss_ = 0, off_ = 0;
    std::map<std::string, Entry> files_;
    Bytes read(uint32_t lba, uint32_t size) const;
    void walk(uint32_t lba, uint32_t size, const std::string &prefix, int depth);
};

// Opens a disc image given as .cue, .bin, .iso, .img or .zip. A zip's image is unpacked into unpack_dir first.
// how: what was opened ("x.cue -> x.bin").
std::unique_ptr<Disc> open_image(const fs::path &path, const fs::path &unpack_dir, std::string *how = nullptr);
