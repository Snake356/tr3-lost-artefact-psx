// Deflate decompression (RFC 1951) and reading .zip archives (the players' disc images often come zipped).
#pragma once
#include <cstdio>
#include <memory>
#include <string>
#include <vector>

#include "files.h"

Bytes inflate_raw(const uint8_t *p, size_t n, size_t size_hint = 0);

struct ZipEntry {
    std::string name;
    uint64_t size = 0, packed = 0, header_offset = 0;
    uint16_t method = 0;
    uint32_t crc = 0;
};

// A zip archive; members are unpacked to a file, streaming (disc images are hundreds of MB).
class ZipArchive {
public:
    explicit ZipArchive(const fs::path &p);
    ~ZipArchive();
    const std::vector<ZipEntry> &entries() const { return entries_; }
    void extract(const ZipEntry &e, const fs::path &dst) const;
    Bytes read(const ZipEntry &e) const;

private:
    FILE *f_ = nullptr;
    fs::path path_;
    std::vector<ZipEntry> entries_;
    uint64_t data_offset(const ZipEntry &e) const;
};
