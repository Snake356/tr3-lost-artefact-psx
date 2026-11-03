#include "ntsc.h"

#include <map>
#include <regex>
#include <vector>

namespace {

// CRC32 of entries 0-79 (F1-F10) of the XA clip table (160 bytes, 4-byte aligned): the same in every
// language and revision, the table is found by it
const uint32_t TABLE_HEAD_CRC = 0x7F0FCCE7;

// CRC32 of the best-times headings (assault course, quad bike track), NUL-terminated strings in the US executable
const std::map<std::string, std::pair<uint32_t, uint32_t>> HEADING_CRC = {
    {"de", {0x5F360B91, 0x9E1D17C8}}, {"fr", {0xE53D09C4, 0xD7239B95}}, {"it", {0x02874E27, 0x6DC5FA9E}},
    {"sp", {0x06D2FB56, 0x73B1D368}}, {"en", {0x25DD3440, 0x38EF40F2}}};
const size_t F14_ENTRY = 111;          // Rev 0 has 0 here (Rev 1 corrected it)
const uint16_t F14_VALUE = 65440;

size_t only_one(const std::vector<size_t> &hits, const std::string &what) {
    if (hits.size() != 1) throw Error(what + ": found " + std::to_string(hits.size()) + " times");
    return hits[0];
}

size_t find_table(const Bytes &d, const std::string &what) {
    std::vector<size_t> hits;
    for (size_t i = 0; i + 160 < d.size(); i += 4)
        if (crc32(&d[i], 160) == TABLE_HEAD_CRC) hits.push_back(i);
    return only_one(hits, what);
}

// The one NUL-delimited string of 4 to 64 bytes with this CRC32.
size_t find_string(const Bytes &d, uint32_t crc, const std::string &what) {
    std::vector<size_t> hits;
    for (size_t i = 0; i < d.size();) {
        if (d[i] != 0) {
            i++;
            continue;
        }
        size_t a = i + 1, b = a;
        while (b < d.size() && d[b] != 0) b++;
        if (b < d.size() && b - a >= 4 && b - a <= 64 && crc32(&d[a], b - a) == crc) hits.push_back(a);
        i = b;
    }
    return only_one(hits, what);
}

size_t find_words(const Bytes &d, const std::vector<uint32_t> &words, const std::string &what) {
    Bytes needle;
    Writer w{needle};
    for (uint32_t x : words) w.u32(x);
    std::vector<size_t> hits;
    for (size_t i = find_bytes(d, needle); i != std::string::npos; i = find_bytes(d, needle, i + 1))
        if (i % 4 == 0) hits.push_back(i);
    return only_one(hits, what);
}

uint32_t ram(size_t offset) { return uint32_t(offset + 0x10000 - 0x800); }    // file offset -> RAM (kuseg)
uint32_t hi(uint32_t a) { return (a + 0x8000) >> 16; }

const char NTSC_CARD[] = "BASLUS-00691TR3G";

Bytes xor_body(const Bytes &dat) {
    uint8_t key = u8(dat, 382);
    Bytes body(dat.begin() + 384, dat.end());
    for (auto &b : body) b ^= key;
    return body;
}

Bytes with_body(const Bytes &dat, Bytes body) {
    uint8_t key = dat[382];
    for (auto &b : body) b ^= key;
    Bytes out(dat.begin(), dat.begin() + 384);
    out.insert(out.end(), body.begin(), body.end());
    return out;
}

bool is_upper(uint8_t c) { return c >= 'A' && c <= 'Z'; }
bool is_digit(uint8_t c) { return c >= '0' && c <= '9'; }

}

Bytes make_ntsc_exe(const Bytes &us_exe, const Bytes &lang_exe, const std::string &lang) {
    Bytes us = us_exe;
    size_t us_table = find_table(us_exe, "XA clip table (US)"), table = find_table(lang_exe, "XA clip table (language)");
    for (size_t k = 120; k < 128; k++)
        if (u16(us_exe, us_table + 2 * k) != u16(lang_exe, table + 2 * k))
            throw Error("tables differ after F15 - not the XA clip table?");
    if (lang != "en")
        std::copy(lang_exe.begin() + long(table + 160), lang_exe.begin() + long(table + 240), us.begin() + long(us_table + 160));
    else if (u16(us, us_table + 2 * F14_ENTRY) == 0)      // English: the US executable's own table
        put16(us, us_table + 2 * F14_ENTRY, F14_VALUE);

    auto en = HEADING_CRC.at("en"), nw = HEADING_CRC.at(lang);
    uint32_t en0 = ram(find_string(us_exe, en.first, "heading en 0")), en1 = ram(find_string(us_exe, en.second, "heading en 1"));
    uint32_t new0 = ram(find_string(us_exe, nw.first, "heading " + lang + " 0"));
    uint32_t new1 = ram(find_string(us_exe, nw.second, "heading " + lang + " 1"));
    size_t assault = find_words(us_exe, {0x24A50000 | (en0 & 0xFFFF)}, "addiu Assault Course");
    size_t quad = find_words(us_exe, {0x3C050000 | hi(en1), 0x24A50000 | (en1 & 0xFFFF)}, "lui/addiu Quadbike Track");
    if (hi(new0) != hi(en0)) throw Error("the lui before the assault course addiu does not fit");
    put32(us, assault, 0x24A50000 | (new0 & 0xFFFF));
    put32(us, quad, 0x3C050000 | hi(new1));
    put32(us, quad + 4, 0x24A50000 | (new1 & 0xFFFF));
    return us;
}

Bytes ntsc_card_name(const Bytes &dat, const std::string &lang) {
    static const std::map<std::string, char> LETTER = {{"en", 'E'}, {"de", 'D'}, {"fr", 'F'}, {"it", 'I'}, {"sp", 'S'}};
    Bytes body = xor_body(dat), card = to_bytes(NTSC_CARD);
    size_t at = find_bytes(body, card);
    if (at == std::string::npos || find_bytes(body, card, at + 1) != std::string::npos)
        throw Error("US memory card name not found");
    body[at + card.size() - 2] = uint8_t(LETTER.at(lang));
    return with_body(dat, body);
}

Bytes rev0_card_names(const Bytes &dat) {
    Bytes body = xor_body(dat);
    // B[A-Z]S[A-Z]{2}[SM]-\d{5}TR[A-Z0-9]{2}
    auto match = [&](size_t i) {
        if (i + 16 > body.size()) return false;
        const uint8_t *p = &body[i];
        if (p[0] != 'B' || !is_upper(p[1]) || p[2] != 'S' || !is_upper(p[3]) || !is_upper(p[4]) || (p[5] != 'S' && p[5] != 'M') || p[6] != '-')
            return false;
        for (int k = 7; k < 12; k++)
            if (!is_digit(p[k])) return false;
        return p[12] == 'T' && p[13] == 'R' && (is_upper(p[14]) || is_digit(p[14])) && (is_upper(p[15]) || is_digit(p[15]));
    };
    int found = 0;
    for (size_t i = 0; i < body.size();) {
        if (match(i)) {
            body[i + 13] = '0';
            found++;
            i += 16;
        } else {
            i++;
        }
    }
    if (!found) throw Error("memory card names not found");
    return with_body(dat, body);
}

std::string ntsc_layout(std::string xml, const std::string &layout) {
    const std::pair<std::string, std::string> rep[] = {
        {"<license file=\"./licenses/licensee.dat\"/>", "<license file=\"./licenses/licensea.dat\"/>"},
        {"source=\"./files/system/" + layout + ".cnf\"", "source=\"./files/system/us.cnf\""},
        {"<dir name=\"PIXUK\">", "<dir name=\"PIXUS\">"},
        {"name=\"TITLEUK.RAW\"", "name=\"TITLEUS.RAW\""},
        {"name=\"LEGALUK.RAW\"", "name=\"LEGALUS.RAW\""},
        {"./files/data/uk/", "./files/data/us/"}};
    for (auto &[old, nw] : rep) {
        if (xml.find(old) == std::string::npos) throw Error("disc/" + layout + ".xml: " + old + " not found");
        for (size_t p = xml.find(old); p != std::string::npos; p = xml.find(old, p + nw.size())) xml.replace(p, old.size(), nw);
    }
    xml = std::regex_replace(xml, std::regex(R"(<file name="SLES_\d{3}\.\d{2}" type="data")"), "<file name=\"SLUS_006.91\" type=\"data\"");
    // NTSC shows 240 lines: the title with its borders
    xml = std::regex_replace(xml, std::regex(R"((art/loadscr/\w+/title\w+)_pal\.raw)"), "$1.raw");
    xml = std::regex_replace(xml, std::regex(R"(\.\./levels/out/\w+rev/)"), "../levels/out/us/");
    return xml;
}
