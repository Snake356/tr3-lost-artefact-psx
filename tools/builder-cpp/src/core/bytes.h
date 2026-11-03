// Byte buffers and little-endian access, as the level and disc formats use them.
#pragma once
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

using Bytes = std::vector<uint8_t>;

struct Error : std::runtime_error {
    using std::runtime_error::runtime_error;
};

inline void check_range(const Bytes &b, size_t o, size_t n) {
    if (o > b.size() || n > b.size() - o)
        throw Error("read past the end of the data (offset " + std::to_string(o) + ")");
}

inline uint8_t u8(const Bytes &b, size_t o) { check_range(b, o, 1); return b[o]; }
inline uint16_t u16(const Bytes &b, size_t o) { check_range(b, o, 2); return uint16_t(b[o] | b[o + 1] << 8); }
inline int16_t s16(const Bytes &b, size_t o) { return int16_t(u16(b, o)); }
inline uint32_t u32(const Bytes &b, size_t o) {
    check_range(b, o, 4);
    return uint32_t(b[o]) | uint32_t(b[o + 1]) << 8 | uint32_t(b[o + 2]) << 16 | uint32_t(b[o + 3]) << 24;
}
inline int32_t s32(const Bytes &b, size_t o) { return int32_t(u32(b, o)); }

inline void put8(Bytes &b, size_t o, uint32_t v) { check_range(b, o, 1); b[o] = uint8_t(v); }
inline void put16(Bytes &b, size_t o, uint32_t v) { check_range(b, o, 2); b[o] = uint8_t(v); b[o + 1] = uint8_t(v >> 8); }
inline void put32(Bytes &b, size_t o, uint32_t v) {
    check_range(b, o, 4);
    for (int i = 0; i < 4; i++) b[o + i] = uint8_t(v >> (8 * i));
}

// Appending writer: w.u16(x).u32(y)...
struct Writer {
    Bytes &b;
    Writer &u8(uint32_t v) { b.push_back(uint8_t(v)); return *this; }
    Writer &u16(uint32_t v) { b.push_back(uint8_t(v)); b.push_back(uint8_t(v >> 8)); return *this; }
    Writer &u32(uint32_t v) { for (int i = 0; i < 4; i++) b.push_back(uint8_t(v >> (8 * i))); return *this; }
    Writer &bytes(const uint8_t *p, size_t n) { b.insert(b.end(), p, p + n); return *this; }
    Writer &bytes(const Bytes &x) { b.insert(b.end(), x.begin(), x.end()); return *this; }
};

inline Bytes slice(const Bytes &b, size_t o, size_t n) {
    check_range(b, o, n);
    return Bytes(b.begin() + o, b.begin() + o + n);
}

inline Bytes concat(std::initializer_list<const Bytes *> parts) {
    size_t n = 0;
    for (auto *p : parts) n += p->size();
    Bytes out;
    out.reserve(n);
    for (auto *p : parts) out.insert(out.end(), p->begin(), p->end());
    return out;
}

inline Bytes from_hex(std::string_view hex) {
    auto nib = [](char c) -> int {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        throw Error("bad hex digit");
    };
    Bytes out;
    out.reserve(hex.size() / 2);
    for (size_t i = 0; i + 1 < hex.size(); i += 2) out.push_back(uint8_t(nib(hex[i]) << 4 | nib(hex[i + 1])));
    return out;
}

inline Bytes to_bytes(std::string_view s) { return Bytes(s.begin(), s.end()); }

// Index of needle in hay at or after start, or npos.
inline size_t find_bytes(const Bytes &hay, const Bytes &needle, size_t start = 0) {
    if (needle.empty() || needle.size() > hay.size()) return std::string::npos;
    for (size_t i = start; i + needle.size() <= hay.size(); i++)
        if (hay[i] == needle[0] && std::memcmp(&hay[i], needle.data(), needle.size()) == 0) return i;
    return std::string::npos;
}

uint32_t crc32(const uint8_t *p, size_t n, uint32_t crc = 0);
inline uint32_t crc32(const Bytes &b) { return crc32(b.data(), b.size()); }
std::string hex8(uint32_t v);   // "%08x"
