#include "bytes.h"

#include <cstdio>

namespace {
struct Table {
    uint32_t t[256];
    Table() {
        for (uint32_t i = 0; i < 256; i++) {
            uint32_t c = i;
            for (int k = 0; k < 8; k++) c = c & 1 ? 0xEDB88320u ^ (c >> 1) : c >> 1;
            t[i] = c;
        }
    }
};
const Table table;
}

uint32_t crc32(const uint8_t *p, size_t n, uint32_t crc) {
    crc = ~crc;
    for (size_t i = 0; i < n; i++) crc = table.t[(crc ^ p[i]) & 0xFF] ^ (crc >> 8);
    return ~crc;
}

std::string hex8(uint32_t v) {
    char s[9];
    std::snprintf(s, sizeof s, "%08x", v);
    return s;
}
