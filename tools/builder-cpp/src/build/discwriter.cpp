#include "discwriter.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <charconv>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>

namespace {

// ---------------------------------------------------------------------------------------------------------------
// A small XML reader: elements, attributes, comments and the declaration. Text content is skipped.

struct XmlElement {
    std::string name;
    std::vector<std::pair<std::string, std::string>> attrs;
    std::vector<XmlElement> children;
    int line = 0;

    const std::string *attr(const char *n) const {
        for (auto &a : attrs)
            if (a.first == n) return &a.second;
        return nullptr;
    }
    const XmlElement *child(const char *n) const {
        for (auto &c : children)
            if (c.name == n) return &c;
        return nullptr;
    }
};

bool iequals(const std::string &a, const char *b) {
    size_t n = std::strlen(b);
    if (a.size() != n) return false;
    for (size_t i = 0; i < n; i++)
        if (std::tolower(uint8_t(a[i])) != std::tolower(uint8_t(b[i]))) return false;
    return true;
}

class XmlReader {
public:
    explicit XmlReader(const std::string &s) : s_(s) {}

    XmlElement parse() {
        XmlElement doc;
        std::vector<XmlElement *> stack{&doc};
        while (p_ < s_.size()) {
            if (s_[p_] != '<') {
                advance(1);
                continue;
            }
            if (s_.compare(p_, 4, "<!--") == 0) {
                skip_past("-->");
            } else if (s_.compare(p_, 2, "<?") == 0) {
                skip_past("?>");
            } else if (s_.compare(p_, 2, "<!") == 0) {
                skip_past(">");
            } else if (s_.compare(p_, 2, "</") == 0) {
                advance(2);
                std::string n = name();
                skip_space();
                expect('>');
                if (stack.size() < 2 || stack.back()->name != n) fail("unexpected </" + n + ">");
                stack.pop_back();
            } else {
                advance(1);
                XmlElement e;
                e.line = line_;
                e.name = name();
                bool closed = false;
                for (;;) {
                    skip_space();
                    if (p_ >= s_.size()) fail("unterminated <" + e.name + ">");
                    if (s_[p_] == '/') {
                        advance(1);
                        expect('>');
                        closed = true;
                        break;
                    }
                    if (s_[p_] == '>') {
                        advance(1);
                        break;
                    }
                    std::string an = name();
                    skip_space();
                    expect('=');
                    skip_space();
                    if (p_ >= s_.size() || (s_[p_] != '"' && s_[p_] != '\'')) fail("attribute " + an + " has no quoted value");
                    char q = s_[p_];
                    advance(1);
                    size_t end = s_.find(q, p_);
                    if (end == std::string::npos) fail("unterminated attribute " + an);
                    std::string v = decode(s_.substr(p_, end - p_));
                    advance(end - p_ + 1);
                    e.attrs.emplace_back(std::move(an), std::move(v));
                }
                stack.back()->children.push_back(std::move(e));
                if (!closed) stack.push_back(&stack.back()->children.back());
            }
        }
        if (stack.size() != 1) fail("<" + stack.back()->name + "> is not closed");
        return doc;
    }

private:
    const std::string &s_;
    size_t p_ = 0;
    int line_ = 1;

    [[noreturn]] void fail(const std::string &what) const {
        throw Error("disc layout XML, line " + std::to_string(line_) + ": " + what);
    }
    void advance(size_t n) {
        for (size_t i = 0; i < n && p_ < s_.size(); i++, p_++)
            if (s_[p_] == '\n') line_++;
    }
    void skip_space() {
        while (p_ < s_.size() && std::isspace(uint8_t(s_[p_]))) advance(1);
    }
    void skip_past(const char *end) {
        size_t e = s_.find(end, p_);
        if (e == std::string::npos) fail("unterminated markup");
        advance(e - p_ + std::strlen(end));
    }
    void expect(char c) {
        if (p_ >= s_.size() || s_[p_] != c) fail(std::string("expected '") + c + "'");
        advance(1);
    }
    std::string name() {
        size_t b = p_;
        while (p_ < s_.size() && (std::isalnum(uint8_t(s_[p_])) || std::strchr("_-:.", s_[p_]) || uint8_t(s_[p_]) >= 0x80))
            advance(1);
        if (p_ == b) fail("expected a name");
        return s_.substr(b, p_ - b);
    }
    std::string decode(const std::string &v) const {
        std::string out;
        for (size_t i = 0; i < v.size(); i++) {
            if (v[i] != '&') {
                out += v[i];
                continue;
            }
            size_t semi = v.find(';', i);
            if (semi == std::string::npos) fail("bad entity in attribute");
            std::string ent = v.substr(i + 1, semi - i - 1);
            if (ent == "amp") out += '&';
            else if (ent == "lt") out += '<';
            else if (ent == "gt") out += '>';
            else if (ent == "quot") out += '"';
            else if (ent == "apos") out += '\'';
            else if (!ent.empty() && ent[0] == '#') {
                uint32_t c = 0;
                bool hex = ent.size() > 1 && (ent[1] == 'x' || ent[1] == 'X');
                std::from_chars(ent.data() + (hex ? 2 : 1), ent.data() + ent.size(), c, hex ? 16 : 10);
                if (c < 0x80) out += char(c);
                else if (c < 0x800) { out += char(0xC0 | c >> 6); out += char(0x80 | (c & 0x3F)); }
                else if (c < 0x10000) { out += char(0xE0 | c >> 12); out += char(0x80 | (c >> 6 & 0x3F)); out += char(0x80 | (c & 0x3F)); }
                else { out += char(0xF0 | c >> 18); out += char(0x80 | (c >> 12 & 0x3F)); out += char(0x80 | (c >> 6 & 0x3F)); out += char(0x80 | (c & 0x3F)); }
            } else fail("unknown entity &" + ent + ";");
            i = semi;
        }
        return out;
    }
};

// ---------------------------------------------------------------------------------------------------------------
// EDC (CRC with polynomial 0xD8018001) and the Reed-Solomon P/Q ECC of CD-ROM sectors.

struct EdcEccTables {
    uint8_t ecc_f[256], ecc_b[256];
    uint32_t edc[256];
    EdcEccTables() {
        for (uint32_t i = 0; i < 256; i++) {
            uint32_t j = (i << 1) ^ (i & 0x80 ? 0x11D : 0);
            ecc_f[i] = uint8_t(j);
            ecc_b[i ^ j] = uint8_t(i);
            uint32_t e = i;
            for (int k = 0; k < 8; k++) e = (e >> 1) ^ (e & 1 ? 0xD8018001u : 0);
            edc[i] = e;
        }
    }
};
const EdcEccTables tables;

void put_edc(const uint8_t *src, size_t len, uint8_t *dest) {
    uint32_t e = 0;
    for (size_t i = 0; i < len; i++) e = (e >> 8) ^ tables.edc[(e ^ src[i]) & 0xFF];
    for (int i = 0; i < 4; i++) dest[i] = uint8_t(e >> (8 * i));
}

void put_ecc(const uint8_t *src, uint32_t major_count, uint32_t minor_count, uint32_t major_mult, uint32_t minor_inc,
             uint8_t *dest) {
    uint32_t len = major_count * minor_count;
    for (uint32_t major = 0; major < major_count; major++) {
        uint32_t index = (major >> 1) * major_mult + (major & 1);
        uint8_t a = 0, b = 0;
        for (uint32_t minor = 0; minor < minor_count; minor++) {
            uint8_t t = src[index];
            index += minor_inc;
            if (index >= len) index -= len;
            a ^= t;
            b ^= t;
            a = tables.ecc_f[a];
        }
        a = tables.ecc_b[tables.ecc_f[a] ^ b];
        dest[major] = a;
        dest[major + major_count] = uint8_t(a ^ b);
    }
}

// ---------------------------------------------------------------------------------------------------------------
// Sector writer. Like mkpsxiso's writer it keeps one sector buffer that is never cleared between sectors and a
// sub-header that stays set until changed; sectors go to the file through a large buffer of consecutive sectors.

constexpr uint32_t kSector = 2352;
constexpr uint32_t kSubData = 0x00080000, kSubStr = 0x00480100, kSubEol = 0x00090000, kSubEof = 0x00890000;
enum class Form { One, Two };

class SectorWriter {
public:
    explicit SectorWriter(const fs::path &p) : path_(p), buf_(size_t(kBufSectors) * kSector) {
        f_ = _wfopen(p.wstring().c_str(), L"wb");
        if (!f_) throw Error("cannot write " + path_str(p));
    }
    ~SectorWriter() {
        if (f_) std::fclose(f_);
    }
    SectorWriter(const SectorWriter &) = delete;
    SectorWriter &operator=(const SectorWriter &) = delete;

    void subheader(uint32_t v) {
        for (int i = 0; i < 4; i++) sub_[i] = sub_[i + 4] = uint8_t(v >> (8 * i));
    }
    void seek(uint32_t lba) { cur_ = lba; }

    // 2048 bytes of user data under the current sub-header, as a mode 2 form 1 sector.
    void data(const uint8_t *d) {
        std::memcpy(sec_ + 16, sub_, 8);
        std::memcpy(sec_ + 24, d, 2048);
        emit(Form::One);
    }
    // 2336 bytes (sub-header included) as they come from an XA/STR stream.
    void raw(const uint8_t *d, Form form) {
        std::memcpy(sec_ + 16, d, 2336);
        emit(form);
    }

    void close() {
        flush();
        bool ok = std::fflush(f_) == 0;
        ok = std::fclose(f_) == 0 && ok;
        f_ = nullptr;
        if (!ok) throw Error("cannot write " + path_str(path_));
    }

private:
    static constexpr uint32_t kBufSectors = 4096;
    fs::path path_;
    FILE *f_ = nullptr;
    Bytes buf_;
    uint32_t buf_start_ = 0, buf_n_ = 0, cur_ = 0;
    uint8_t sec_[kSector] = {};
    uint8_t sub_[8] = {};

    void emit(Form form) {
        prepare(form);
        if (buf_n_ && cur_ != buf_start_ + buf_n_) flush();
        if (!buf_n_) buf_start_ = cur_;
        std::memcpy(&buf_[size_t(buf_n_) * kSector], sec_, kSector);
        buf_n_++;
        cur_++;
        if (buf_n_ == kBufSectors) flush();
    }
    void flush() {
        if (!buf_n_) return;
        if (_fseeki64(f_, int64_t(buf_start_) * kSector, SEEK_SET) != 0 ||
            std::fwrite(buf_.data(), kSector, buf_n_, f_) != buf_n_)
            throw Error("cannot write " + path_str(path_));
        buf_n_ = 0;
    }
    void prepare(Form form) {
        std::memset(sec_, 0xFF, 12);
        sec_[0] = sec_[11] = 0;
        uint32_t addr = 150 + cur_;
        auto bcd = [](uint32_t v) { return uint8_t(16 * (v / 10) + v % 10); };
        sec_[12] = bcd(addr / 75 / 60);
        sec_[13] = bcd(addr / 75 % 60);
        sec_[14] = bcd(addr % 75);
        sec_[15] = 2;
        if (form == Form::One) {
            put_edc(sec_ + 16, 0x808, sec_ + 0x818);
            uint8_t hdr[4];
            std::memcpy(hdr, sec_ + 12, 4);
            std::memset(sec_ + 12, 0, 4);   // the ECC of a mode 2 sector is taken with a zero header
            put_ecc(sec_ + 12, 86, 24, 2, 86, sec_ + 0x81C);
            put_ecc(sec_ + 12, 52, 43, 86, 88, sec_ + 0x8C8);
            std::memcpy(sec_ + 12, hdr, 4);
        } else {
            put_edc(sec_ + 16, 0x91C, sec_ + 16 + 2332);
        }
    }
};

// ---------------------------------------------------------------------------------------------------------------
// The directory tree, laid out as mkpsxiso lays it out.

enum class Kind { File, Dir, Str, StrData };   // Str: mixed form 1/2 stream (xa, str); StrData: 2048-byte stream

struct Dir;
struct Entry {
    std::string id;   // "NAME.EXT;1" for files, "NAME" for directories, empty for a dummy
    Kind kind = Kind::File;
    uint32_t length = 0;
    uint32_t lba = 0;
    fs::path src;
    std::unique_ptr<Dir> sub;
    std::array<uint8_t, 7> date{};
};

struct Dir {
    std::vector<Entry> entries;
    uint32_t record_lba = 0;
    Dir *parent = nullptr;
};

struct Layout {
    bool xa = true;   // XA attributes in the directory records
    std::array<uint8_t, 7> build_utc{}, build_local{};
};

std::array<uint8_t, 7> iso_date(const std::tm &t) {
    return {uint8_t(t.tm_year), uint8_t(t.tm_mon + 1), uint8_t(t.tm_mday), uint8_t(t.tm_hour), uint8_t(t.tm_min),
            uint8_t(t.tm_sec), 0};
}

std::tm utc_tm(std::time_t t) {
    std::tm r{};
    if (gmtime_s(&r, &t) != 0) throw Error("bad time stamp");
    return r;
}

std::tm local_tm(std::time_t t) {
    std::tm r{};
    if (localtime_s(&r, &t) != 0) throw Error("bad time stamp");
    return r;
}

std::string upper_ascii(std::string s) {
    for (char &c : s) c = char(std::toupper(uint8_t(c)));
    return s;
}

uint32_t ceil_div(uint64_t a, uint32_t b) { return uint32_t((a + b - 1) / b); }

// Bytes of the directory record as mkpsxiso reserves them (rounded up to whole sectors).
uint32_t dir_record_len(const Layout &L, const Dir &d) {
    uint32_t n = 68 + (L.xa ? 28 : 0);
    for (auto &e : d.entries) {
        if (e.id.empty()) continue;
        uint32_t len = 33 + 2 * uint32_t((e.id.size() + 1) / 2);
        if (e.id.size() % 2 == 0) len++;
        if (L.xa) len += 14;
        n += len;
    }
    return 2048 * ceil_div(n, 2048);
}

uint32_t sectors_of(const Entry &e) {
    if (e.kind == Kind::File || e.kind == Kind::StrData) return ceil_div(e.length, 2048);
    if (e.kind == Kind::Str) return ceil_div(e.length, 2336);
    return 0;
}

uint32_t file_system_size(const Layout &L, Dir &d, uint32_t lba) {
    d.record_lba = lba;
    lba += dir_record_len(L, d) / 2048;
    for (auto &e : d.entries) lba = e.sub ? file_system_size(L, *e.sub, lba) : lba + sectors_of(e);
    return lba;
}

uint32_t assign_lbas(const Layout &L, Dir &d, uint32_t lba) {
    d.record_lba = lba;
    lba += dir_record_len(L, d) / 2048;
    for (auto &e : d.entries) {
        e.lba = lba;
        if (e.sub) {
            lba = assign_lbas(L, *e.sub, lba);
            e.length = dir_record_len(L, *e.sub);
        } else {
            lba += sectors_of(e);
        }
    }
    return lba;
}

uint32_t path_table_len_sub(const Entry &e) {
    uint32_t n = 8 + 2 * uint32_t((e.id.size() + 1) / 2);
    for (auto &c : e.sub->entries)
        if (c.kind == Kind::Dir) n += path_table_len_sub(c);
    return n;
}

uint32_t path_table_len(const Dir &root) {
    uint32_t n = 10;
    for (auto &e : root.entries)
        if (e.kind == Kind::Dir) n += path_table_len_sub(e);
    return n;
}

// Directories first sorted in, dummies to the end, the rest by identifier (mkpsxiso's stable bubble sort).
void sort_entries(Dir &d) {
    if (d.entries.size() < 2) return;
    for (auto &e : d.entries)
        if (e.kind == Kind::Dir) sort_entries(*e.sub);
    std::stable_partition(d.entries.begin(), d.entries.end(), [](const Entry &e) { return !e.id.empty(); });
    auto named_end = std::find_if(d.entries.begin(), d.entries.end(), [](const Entry &e) { return e.id.empty(); });
    std::stable_sort(d.entries.begin(), named_end, [](const Entry &a, const Entry &b) { return a.id < b.id; });
}

class Builder {
public:
    Builder(const Layout &L, SectorWriter &w) : L_(L), w_(w), io_(size_t(kChunk)) {}

    void write_files(const Dir &d) {
        for (auto &e : d.entries) {
            w_.seek(e.lba);
            if (e.kind == Kind::Dir) {
                write_files(*e.sub);
            } else if (e.src.empty()) {   // a dummy: zero sectors
                std::array<uint8_t, 2048> zero{};
                uint32_t n = e.length / 2048;
                w_.subheader(kSubData);
                for (uint32_t i = 0; i < n; i++) {
                    if (i == n - 1) w_.subheader(kSubEof);
                    w_.data(zero.data());
                }
            } else {
                write_stream(e);
            }
        }
    }

    void write_dir_records(const Dir &d, uint32_t parent_lba) {
        if (parent_lba == 0) parent_lba = d.record_lba;
        write_dir_record(d, parent_lba);
        for (auto &e : d.entries)
            if (e.kind == Kind::Dir) write_dir_records(*e.sub, d.record_lba);
    }

private:
    // a multiple of both 2048 and 2336, so a chunk always holds whole sectors
    static constexpr uint32_t kChunk = 2048 * 2336;
    const Layout &L_;
    SectorWriter &w_;
    Bytes io_;

    // Writes a source file. Like mkpsxiso it writes size/sector_size + 1 sectors: when the size is a whole number
    // of sectors, one more zero sector follows (later writes cover it, except at the very end of the image).
    void write_stream(const Entry &e) {
        FILE *f = _wfopen(e.src.wstring().c_str(), L"rb");
        if (!f) throw Error("cannot read " + path_str(e.src));
        const uint32_t ss = e.kind == Kind::Str ? 2336 : 2048;
        std::array<uint8_t, 2336> last{};
        if (e.kind == Kind::StrData) w_.subheader(kSubStr);
        else if (e.kind == Kind::File) w_.subheader(kSubData);
        for (;;) {
            size_t got = std::fread(io_.data(), 1, kChunk, f);
            if (std::ferror(f)) {
                std::fclose(f);
                throw Error("cannot read " + path_str(e.src));
            }
            size_t full = got / ss;
            for (size_t i = 0; i < full; i++) put(e.kind, &io_[i * ss]);
            if (got < kChunk) {
                last.fill(0);
                std::memcpy(last.data(), &io_[full * ss], got - full * ss);
                if (e.kind == Kind::File) w_.subheader(kSubEof);
                put(e.kind, last.data());
                break;
            }
        }
        std::fclose(f);
        if (e.kind == Kind::StrData) w_.subheader(kSubData);
    }

    void put(Kind k, const uint8_t *d) {
        if (k == Kind::Str) w_.raw(d, (d[2] & 0x20) ? Form::Two : Form::One);
        else w_.data(d);
    }

    // one directory record entry: 33 bytes, the identifier (padded to even length), the XA attributes
    size_t record(uint8_t *p, uint32_t lba, uint32_t size, const std::array<uint8_t, 7> &date, uint8_t flags,
                  const std::string &id, uint16_t xa_attr) const {
        std::memset(p, 0, 128);
        auto pair32 = [&](size_t o, uint32_t v) {
            for (int i = 0; i < 4; i++) p[o + i] = uint8_t(v >> (8 * i)), p[o + 7 - i] = uint8_t(v >> (8 * i));
        };
        pair32(2, lba);
        pair32(10, size);
        std::memcpy(p + 18, date.data(), 7);
        p[25] = flags;
        p[28] = 1;
        p[31] = 1;
        p[32] = uint8_t(id.size());
        std::memcpy(p + 33, id.data(), id.size());
        size_t n = 33 + id.size();
        if (id.size() % 2 == 0) n++;
        if (L_.xa) {
            p[n + 4] = uint8_t(xa_attr);
            p[n + 5] = uint8_t(xa_attr >> 8);
            p[n + 6] = 'X';
            p[n + 7] = 'A';
            n += 14;
        }
        p[0] = uint8_t(n);
        return n;
    }

    void write_dir_record(const Dir &d, uint32_t parent_lba) {
        w_.seek(d.record_lba);
        std::array<uint8_t, 2048> sec{};
        size_t pos = 0;
        uint8_t rec[128];
        // "." and ".." have a one-byte identifier 0x00 / 0x01
        size_t n = record(rec, d.record_lba, dir_record_len(L_, d), L_.build_utc, 2, std::string(1, '\0'), 0x558d);
        std::memcpy(&sec[pos], rec, n);
        pos += n;
        uint32_t plen = dir_record_len(L_, d.parent ? *d.parent : d);
        n = record(rec, parent_lba, plen, L_.build_utc, 2, std::string(1, '\1'), 0x558d);
        std::memcpy(&sec[pos], rec, n);
        pos += n;
        for (auto &e : d.entries) {
            if (e.id.empty()) continue;
            uint32_t size = e.length;
            uint16_t attr = 0x550d;
            if (e.kind == Kind::Str) size = 2048 * ceil_div(e.length, 2336), attr = 0x553d;
            else if (e.kind == Kind::StrData) size = 2048 * ceil_div(e.length, 2048);
            else if (e.kind == Kind::Dir) attr = 0x558d;
            n = record(rec, e.lba, size, e.date, e.kind == Kind::Dir ? 2 : 0, e.id, attr);
            if (pos + n > 2047) {   // mkpsxiso's test: an entry ending exactly at 2048 also moves on
                w_.subheader(kSubData);
                w_.data(sec.data());
                sec.fill(0);
                pos = 0;
            }
            std::memcpy(&sec[pos], rec, n);
            pos += n;
        }
        w_.subheader(kSubEof);
        w_.data(sec.data());
    }
};

// Path table: the root, then each directory level in mkpsxiso's order.
struct PathEntry {
    const Entry *e;
    uint16_t parent;
    std::vector<PathEntry> sub;
};

void gen_path_entries(std::vector<PathEntry> &out, const Dir &d, uint16_t parent, uint16_t &index) {
    for (auto &e : d.entries)
        if (e.kind == Kind::Dir) out.push_back({&e, parent, {}}), index++;
    uint16_t first = uint16_t(index - out.size() + 1);
    for (size_t i = 0; i < out.size(); i++) gen_path_entries(out[i].sub, *out[i].e->sub, uint16_t(first + i), index);
}

void put_path_entries(uint8_t *&p, const std::vector<PathEntry> &list, bool msb) {
    for (auto &pe : list) {
        uint32_t lba = pe.e->sub->record_lba;
        p[0] = uint8_t(pe.e->id.size());
        p[1] = 0;
        for (int i = 0; i < 4; i++) p[2 + i] = uint8_t(lba >> (8 * (msb ? 3 - i : i)));
        p[6] = uint8_t(msb ? pe.parent >> 8 : pe.parent);
        p[7] = uint8_t(msb ? pe.parent : pe.parent >> 8);
        std::memcpy(p + 8, pe.e->id.data(), pe.e->id.size());
        p += 8 + 2 * ((pe.e->id.size() + 1) / 2);
    }
    for (auto &pe : list) put_path_entries(p, pe.sub, msb);
}

void put_path_table(uint8_t *p, const Dir &root, bool msb) {
    std::vector<PathEntry> list;
    uint16_t index = 1;
    gen_path_entries(list, root, 1, index);
    p[0] = 1;
    p[1] = 0;
    for (int i = 0; i < 4; i++) p[2 + i] = uint8_t(root.record_lba >> (8 * (msb ? 3 - i : i)));
    p[6] = msb ? 0 : 1;
    p[7] = msb ? 1 : 0;
    p[8] = p[9] = 0;
    p += 10;
    put_path_entries(p, list, msb);
}

struct Identifiers {
    const std::string *system = nullptr, *volume = nullptr, *volume_set = nullptr, *publisher = nullptr,
                      *preparer = nullptr, *application = nullptr, *copyright = nullptr;
};

void put_id(uint8_t *field, size_t size, const std::string *s) {
    std::memset(field, ' ', size);
    if (s)
        for (size_t i = 0; i < s->size() && i < size; i++) field[i] = uint8_t(std::toupper(uint8_t((*s)[i])));
}

void write_descriptors(SectorWriter &w, const Layout &L, const Identifiers &ids, const Dir &root, uint32_t image_len,
                       const std::tm &local) {
    const uint32_t pt_len = path_table_len(root);
    const uint32_t pt_sectors = ceil_div(pt_len, 2048);
    std::array<uint8_t, 2048> d{};
    auto pair16 = [&](size_t o, uint32_t v) { d[o] = uint8_t(v); d[o + 1] = uint8_t(v >> 8); d[o + 2] = uint8_t(v >> 8); d[o + 3] = uint8_t(v); };
    auto le32 = [&](size_t o, uint32_t v) { for (int i = 0; i < 4; i++) d[o + i] = uint8_t(v >> (8 * i)); };
    auto be32 = [&](size_t o, uint32_t v) { for (int i = 0; i < 4; i++) d[o + i] = uint8_t(v >> (8 * (3 - i))); };
    auto pair32 = [&](size_t o, uint32_t v) { le32(o, v); be32(o + 4, v); };

    d[0] = 1;
    std::memcpy(&d[1], "CD001", 5);
    d[6] = 1;
    put_id(&d[8], 32, ids.system);
    put_id(&d[40], 32, ids.volume);
    pair32(80, image_len);
    pair16(120, 1);
    pair16(124, 1);
    pair16(128, 2048);
    pair32(132, pt_len);
    le32(140, 18);
    le32(144, 18 + pt_sectors);
    be32(148, 18 + pt_sectors + 1);   // mkpsxiso's offsets for the M tables (the same as ISO 9660 for one sector)
    be32(152, 18 + pt_sectors + 1 + pt_sectors);
    // root directory record
    d[156] = 34;
    pair32(158, 18 + pt_sectors * 4);
    pair32(166, dir_record_len(L, root));
    std::memcpy(&d[174], L.build_local.data(), 7);
    d[181] = 2;
    pair16(184, 1);
    d[188] = 1;
    put_id(&d[190], 128, ids.volume_set);
    put_id(&d[318], 128, ids.publisher);
    static const char preparer[] =
        "DISC IMAGE CREATED WITH MKPSXISO BY LAMEGUY64 OF MEIDO-TEK PRODUCTIONS HTTPS://GITHUB.COM/LAMEGUY64/MKPSXISO";
    std::memset(&d[446], ' ', 128);
    std::memcpy(&d[446], preparer, sizeof preparer - 1);
    if (ids.preparer)
        for (size_t i = 0; i < ids.preparer->size() && i < 128; i++) d[446 + i] = uint8_t(std::toupper(uint8_t((*ids.preparer)[i])));
    put_id(&d[574], 128, ids.application);
    put_id(&d[702], 37, ids.copyright);
    put_id(&d[739], 37, nullptr);
    put_id(&d[776], 37, nullptr);
    // creation and modification date as mkpsxiso writes them: local time, the month counted from 0
    char date[24];
    std::snprintf(date, sizeof date, "%04d%02d%02d%02d%02d%02d00", local.tm_year + 1900, local.tm_mon, local.tm_mday,
                  local.tm_hour, local.tm_min, local.tm_sec);
    std::memcpy(&d[813], date, std::min<size_t>(16, std::strlen(date)));
    std::memcpy(&d[830], date, std::min<size_t>(16, std::strlen(date)));
    std::memset(&d[847], '0', 16);
    std::memset(&d[864], '0', 16);
    d[881] = 1;
    if (L.xa) std::memcpy(&d[883 + 141], "CD-XA001", 8);

    w.seek(16);
    w.subheader(kSubEol);
    w.data(d.data());
    d.fill(0);
    d[0] = 255;
    std::memcpy(&d[1], "CD001", 5);
    d[6] = 1;
    w.subheader(kSubEof);
    w.data(d.data());

    Bytes table(size_t(pt_sectors) * 2048);
    for (bool msb : {false, true}) {
        put_path_table(table.data(), root, msb);
        for (int copy = 0; copy < 2; copy++) {
            w.subheader(kSubData);
            for (uint32_t i = 0; i < pt_sectors; i++) {
                if (i == pt_sectors - 1) w.subheader(kSubEof);
                w.data(&table[size_t(i) * 2048]);
            }
        }
    }
    w.subheader(kSubData);
}

class LayoutReader {
public:
    explicit LayoutReader(Layout &L) : L_(L) {}

    void read_dir(const XmlElement &x, Dir &d) {
        for (auto &c : x.children) {
            if (iequals(c.name, "file")) {
                add_file(c, d);
            } else if (iequals(c.name, "dummy")) {
                const std::string *s = c.attr("sectors");
                int n = s ? std::atoi(s->c_str()) : 0;
                if (n < 0) fail(c, "bad dummy size");
                Entry e;
                e.length = uint32_t(n) * 2048;
                d.entries.push_back(std::move(e));
            } else if (iequals(c.name, "dir")) {
                const std::string *name = c.attr("name");
                if (!name || name->empty()) fail(c, "<dir> has no name");
                if (name->size() > 12) fail(c, "directory name " + *name + " is longer than 12 characters");
                std::string id = upper_ascii(*name);
                for (auto &e : d.entries)
                    if (e.kind == Kind::Dir && e.id == id) fail(c, "duplicate directory " + *name);
                Entry e;
                e.id = id;
                e.kind = Kind::Dir;
                e.sub = std::make_unique<Dir>();
                e.sub->parent = &d;
                e.length = 2048;
                e.date = L_.build_utc;
                d.entries.push_back(std::move(e));
                read_dir(c, *d.entries.back().sub);
            } else {
                fail(c, "unsupported element <" + c.name + ">");
            }
        }
    }

private:
    Layout &L_;

    [[noreturn]] static void fail(const XmlElement &x, const std::string &what) {
        throw Error("disc layout XML, line " + std::to_string(x.line) + ": " + what);
    }

    void add_file(const XmlElement &c, Dir &d) {
        const std::string *name = c.attr("name"), *source = c.attr("source"), *type = c.attr("type");
        if (!name || !source) fail(c, "<file> needs name and source");
        if (name->find_first_of("/\\") != std::string::npos) fail(c, "file name " + *name + " is a path");
        if (name->size() > 12) fail(c, "file name " + *name + " is longer than 12 characters");
        Entry e;
        e.src = u8path(*source);
        std::error_code ec;
        auto size = fs::file_size(e.src, ec);
        if (ec || !fs::is_regular_file(e.src)) fail(c, "file not found: " + *source);
        if (size > 0x7FFFFFFF) fail(c, *source + " is too large");
        e.length = uint32_t(size);
        if (!type || iequals(*type, "data")) {
            e.kind = Kind::File;
        } else if (iequals(*type, "xa") || iequals(*type, "str") || iequals(*type, "mixed")) {
            e.kind = Kind::Str;
            Bytes head(4, 0);
            if (FILE *f = _wfopen(e.src.wstring().c_str(), L"rb")) {
                size_t got = std::fread(head.data(), 1, 4, f);
                std::fclose(f);
                if (got == 4 && std::memcmp(head.data(), "RIFF", 4) == 0) fail(c, *source + " is a WAV or is not properly ripped");
            }
            if (size % 2336 != 0) {
                if (size % 2048 == 0) e.kind = Kind::StrData;
                else fail(c, *source + " is not a multiple of 2336 or 2048 bytes");
            }
        } else {
            fail(c, "unsupported file type " + *type);
        }
        e.id = upper_ascii(*name) + ";1";
        for (auto &o : d.entries)
            if (o.kind == Kind::File && iequals(o.id, e.id.c_str())) fail(c, "duplicate file " + *name);
        auto mtime = fs::last_write_time(e.src, ec);
        if (ec) fail(c, "cannot read the date of " + *source);
        auto sys = std::chrono::clock_cast<std::chrono::system_clock>(mtime);
        std::time_t t = std::chrono::system_clock::to_time_t(std::chrono::floor<std::chrono::seconds>(sys));
        e.date = iso_date(utc_tm(t));
        d.entries.push_back(std::move(e));
    }
};

}

void write_disc_image(const std::string &xml_text, const fs::path &out_bin, std::time_t build_time) {
    XmlElement doc = XmlReader(xml_text).parse();
    const XmlElement *project = doc.child("iso_project");
    if (!project) throw Error("disc layout: no <iso_project> element");
    const XmlElement *track = project->child("track");
    if (!track) throw Error("disc layout: no <track> element");
    for (auto &c : project->children)
        if (c.name == "track" && &c != track) throw Error("disc layout: only one data track is supported");
    const std::string *track_type = track->attr("type");
    if (!track_type || !iequals(*track_type, "data")) throw Error("disc layout: the track is not a data track");

    Layout L;
    if (const std::string *v = project->attr("no_xa")) {
        int n = 0;
        std::from_chars(v->data(), v->data() + v->size(), n);
        L.xa = n == 0;
    }
    const std::tm utc = utc_tm(build_time), local = local_tm(build_time);
    L.build_utc = iso_date(utc);
    L.build_local = iso_date(local);

    Identifiers ids;
    if (const XmlElement *x = track->child("identifiers")) {
        static const std::string playstation = "PLAYSTATION", copyleft = "COPYLEFTED";
        ids.system = x->attr("system") ? x->attr("system") : &playstation;
        ids.application = x->attr("application") ? x->attr("application") : &playstation;
        ids.copyright = x->attr("copyright") ? x->attr("copyright") : &copyleft;
        ids.volume = x->attr("volume");
        // mkpsxiso reads these two under the names "volumeset" and "datapreparer"
        ids.volume_set = x->attr("volumeset");
        ids.preparer = x->attr("datapreparer");
        ids.publisher = x->attr("publisher");
    }

    Bytes license;
    if (const XmlElement *x = track->child("license")) {
        const std::string *f = x->attr("file");
        if (!f || f->empty()) throw Error("disc layout: <license> has no file");
        license = read_file(u8path(*f));
        license.resize(28032, 0);
    }

    const XmlElement *tree = track->child("directory_tree");
    if (!tree) throw Error("disc layout: no <directory_tree>");
    Dir root;
    LayoutReader(L).read_dir(*tree, root);

    const uint32_t pt_sectors = ceil_div(path_table_len(root), 2048);
    const uint32_t image_len = file_system_size(L, root, 16 + pt_sectors * 4);
    assign_lbas(L, root, 18 + pt_sectors * 4);

    // the cue sheet
    const std::string *cue_name = project->attr("cue_sheet");
    if (cue_name) {
        if (cue_name->empty()) throw Error("disc layout: cue_sheet is blank");
        const std::string *image_name = project->attr("image_name");
        std::string bin_name = image_name ? *image_name : path_str(out_bin.filename());
        std::string cue = "FILE \"" + bin_name + "\" BINARY\r\n  TRACK 01 MODE2/2352\r\n    INDEX 01 00:00:00\r\n";
        write_file(out_bin.parent_path() / u8path(*cue_name), Bytes(cue.begin(), cue.end()));
    }

    if (out_bin.has_parent_path()) fs::create_directories(out_bin.parent_path());
    SectorWriter w(out_bin);
    {
        std::array<uint8_t, 2048> zero{};
        w.subheader(kSubData);
        for (uint32_t i = 0; i < 18 + pt_sectors * 4; i++) w.data(zero.data());
    }
    Builder b(L, w);
    b.write_files(root);
    sort_entries(root);
    b.write_dir_records(root, 0);
    write_descriptors(w, L, ids, root, image_len, local);
    if (!license.empty()) {
        w.seek(0);
        for (int i = 0; i < 12; i++) w.raw(&license[size_t(i) * 2336], Form::One);
        w.subheader(0x00200000);
        for (int i = 0; i < 4; i++) w.data(license.data());   // mkpsxiso repeats the license's first 2048 bytes here
        w.subheader(kSubData);
    }
    w.close();
}
