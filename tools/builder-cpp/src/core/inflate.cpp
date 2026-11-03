#include "inflate.h"

#include <algorithm>
#include <functional>

namespace {

using Source = std::function<size_t(uint8_t *, size_t)>;
using Sink = std::function<void(const uint8_t *, size_t)>;

class BitReader {
public:
    explicit BitReader(Source src) : src_(std::move(src)) {}

    uint32_t peek(int n) {
        while (count_ < n) {
            if (pos_ == len_) {
                len_ = src_(buf_, sizeof buf_);
                pos_ = 0;
                if (len_ == 0) {          // past the end: zero bits; reading them is caught by the caller
                    eof_bits_ += 8;
                    count_ += 8;
                    continue;
                }
            }
            bits_ |= uint64_t(buf_[pos_++]) << count_;
            count_ += 8;
        }
        return uint32_t(bits_ & ((uint64_t(1) << n) - 1));
    }
    void skip(int n) {
        bits_ >>= n;
        count_ -= n;
        if (eof_bits_ > count_) throw Error("deflate data ends too early");
    }
    uint32_t bits(int n) {
        if (n == 0) return 0;
        uint32_t v = peek(n);
        skip(n);
        return v;
    }
    void align() { skip(count_ % 8); }
    uint8_t byte() { return uint8_t(bits(8)); }

private:
    Source src_;
    uint8_t buf_[65536];
    size_t pos_ = 0, len_ = 0;
    uint64_t bits_ = 0;
    int count_ = 0, eof_bits_ = 0;
};

// A canonical Huffman code as one lookup table over the next maxlen bits: entry = length << 9 | symbol.
struct Huffman {
    std::vector<uint16_t> table;
    int maxlen = 0;

    void build(const uint8_t *lens, int n) {
        int count[16] = {0};
        for (int i = 0; i < n; i++) count[lens[i]]++;
        count[0] = 0;
        maxlen = 0;
        for (int l = 1; l < 16; l++) if (count[l]) maxlen = l;
        if (maxlen == 0) maxlen = 1;
        table.assign(size_t(1) << maxlen, 0);
        int next[16], code = 0;
        for (int l = 1; l < 16; l++) {
            code = (code + count[l - 1]) << 1;
            next[l] = code;
        }
        for (int s = 0; s < n; s++) {
            int l = lens[s];
            if (!l) continue;
            int c = next[l]++, r = 0;
            for (int i = 0; i < l; i++) r |= ((c >> i) & 1) << (l - 1 - i);
            for (int k = r; k < (1 << maxlen); k += 1 << l) table[size_t(k)] = uint16_t(l << 9 | s);
        }
    }
    int decode(BitReader &in) const {
        uint16_t e = table[in.peek(maxlen)];
        if (!(e >> 9)) throw Error("bad deflate code");
        in.skip(e >> 9);
        return e & 0x1FF;
    }
};

const uint16_t LEN_BASE[29] = {3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31, 35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258};
const uint8_t LEN_EXTRA[29] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0};
const uint16_t DIST_BASE[30] = {1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193, 257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577};
const uint8_t DIST_EXTRA[30] = {0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13};

class Inflater {
public:
    Inflater(Source src, Sink sink) : in_(std::move(src)), sink_(std::move(sink)), win_(1 << 16) {}

    void run() {
        bool last;
        do {
            last = in_.bits(1);
            int type = int(in_.bits(2));
            if (type == 0) stored();
            else if (type == 1) fixed();
            else if (type == 2) dynamic();
            else throw Error("bad deflate block");
        } while (!last);
        flush();
    }

private:
    BitReader in_;
    Sink sink_;
    std::vector<uint8_t> win_;
    size_t pos_ = 0, flushed_ = 0;

    void put(uint8_t b) {
        win_[pos_++ & 0xFFFF] = b;
        if (pos_ - flushed_ >= 32768) flush();
    }
    void flush() {
        while (flushed_ < pos_) {
            size_t at = flushed_ & 0xFFFF, n = std::min(pos_ - flushed_, size_t(0x10000) - at);
            sink_(&win_[at], n);
            flushed_ += n;
        }
    }
    void stored() {
        in_.align();
        uint32_t len = in_.bits(16), nlen = in_.bits(16);
        if ((len ^ 0xFFFF) != nlen) throw Error("bad stored deflate block");
        while (len--) put(in_.byte());
    }
    void codes(const Huffman &lit, const Huffman &dist) {
        for (;;) {
            int s = lit.decode(in_);
            if (s < 256) {
                put(uint8_t(s));
                continue;
            }
            if (s == 256) return;
            s -= 257;
            if (s >= 29) throw Error("bad deflate length");
            int len = LEN_BASE[s] + int(in_.bits(LEN_EXTRA[s]));
            int d = dist.decode(in_);
            if (d >= 30) throw Error("bad deflate distance");
            size_t back = DIST_BASE[d] + in_.bits(DIST_EXTRA[d]);
            if (back > pos_) throw Error("deflate distance too far back");
            while (len--) put(win_[(pos_ - back) & 0xFFFF]);
        }
    }
    void fixed() {
        uint8_t l[288], d[30];
        std::fill(l, l + 144, uint8_t(8));
        std::fill(l + 144, l + 256, uint8_t(9));
        std::fill(l + 256, l + 280, uint8_t(7));
        std::fill(l + 280, l + 288, uint8_t(8));
        std::fill(d, d + 30, uint8_t(5));
        Huffman lit, dist;
        lit.build(l, 288);
        dist.build(d, 30);
        codes(lit, dist);
    }
    void dynamic() {
        static const uint8_t ORDER[19] = {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};
        int hlit = int(in_.bits(5)) + 257, hdist = int(in_.bits(5)) + 1, hclen = int(in_.bits(4)) + 4;
        uint8_t cl[19] = {0};
        for (int i = 0; i < hclen; i++) cl[ORDER[i]] = uint8_t(in_.bits(3));
        Huffman clh;
        clh.build(cl, 19);
        uint8_t lens[320] = {0};
        for (int i = 0; i < hlit + hdist;) {
            int s = clh.decode(in_);
            if (s < 16) {
                lens[i++] = uint8_t(s);
                continue;
            }
            int rep, val = 0;
            if (s == 16) {
                if (i == 0) throw Error("bad deflate code lengths");
                val = lens[i - 1];
                rep = 3 + int(in_.bits(2));
            } else if (s == 17) {
                rep = 3 + int(in_.bits(3));
            } else {
                rep = 11 + int(in_.bits(7));
            }
            if (i + rep > hlit + hdist) throw Error("bad deflate code lengths");
            while (rep--) lens[i++] = uint8_t(val);
        }
        Huffman lit, dist;
        lit.build(lens, hlit);
        dist.build(lens + hlit, hdist);
        codes(lit, dist);
    }
};

}

Bytes inflate_raw(const uint8_t *p, size_t n, size_t size_hint) {
    Bytes out;
    out.reserve(size_hint);
    size_t at = 0;
    Inflater(
        [&](uint8_t *buf, size_t want) {
            size_t k = std::min(want, n - at);
            std::copy(p + at, p + at + k, buf);
            at += k;
            return k;
        },
        [&](const uint8_t *q, size_t k) { out.insert(out.end(), q, q + k); })
        .run();
    return out;
}

ZipArchive::ZipArchive(const fs::path &p) : path_(p) {
    f_ = _wfopen(p.wstring().c_str(), L"rb");
    if (!f_) throw Error("cannot read " + path_str(p));
    _fseeki64(f_, 0, SEEK_END);
    int64_t size = _ftelli64(f_);
    int64_t tail = std::min<int64_t>(size, 65536 + 22);
    Bytes t(static_cast<size_t>(tail));
    _fseeki64(f_, size - tail, SEEK_SET);
    if (fread(t.data(), 1, t.size(), f_) != t.size()) throw Error("cannot read " + path_str(p));
    size_t eocd = std::string::npos;
    for (size_t i = t.size() - 22 + 1; i-- > 0;)
        if (u32(t, i) == 0x06054b50) {
            eocd = i;
            break;
        }
    if (eocd == std::string::npos) throw Error(path_str(p.filename()) + ": not a zip archive");
    uint64_t count = u16(t, eocd + 10), cd_size = u32(t, eocd + 12), cd_off = u32(t, eocd + 16);
    if (eocd >= 20 && u32(t, eocd - 20) == 0x07064b50) {     // Zip64 end of central directory locator
        uint64_t rec = uint64_t(u32(t, eocd - 12)) | uint64_t(u32(t, eocd - 8)) << 32;
        Bytes r(56);
        _fseeki64(f_, int64_t(rec), SEEK_SET);
        if (fread(r.data(), 1, r.size(), f_) == r.size() && u32(r, 0) == 0x06064b50) {
            auto q = [&](size_t o) { return uint64_t(u32(r, o)) | uint64_t(u32(r, o + 4)) << 32; };
            count = q(32);
            cd_size = q(40);
            cd_off = q(48);
        }
    }
    Bytes cd(static_cast<size_t>(cd_size));
    _fseeki64(f_, int64_t(cd_off), SEEK_SET);
    if (fread(cd.data(), 1, cd.size(), f_) != cd.size()) throw Error(path_str(p.filename()) + ": damaged zip archive");
    size_t o = 0;
    for (uint64_t i = 0; i < count; i++) {
        if (u32(cd, o) != 0x02014b50) throw Error(path_str(p.filename()) + ": damaged zip archive");
        ZipEntry e;
        e.method = u16(cd, o + 10);
        e.crc = u32(cd, o + 16);
        e.packed = u32(cd, o + 20);
        e.size = u32(cd, o + 24);
        size_t nl = u16(cd, o + 28), xl = u16(cd, o + 30), cl = u16(cd, o + 32);
        e.header_offset = u32(cd, o + 42);
        e.name.assign(reinterpret_cast<const char *>(&cd[o + 46]), nl);
        for (size_t x = o + 46 + nl; x + 4 <= o + 46 + nl + xl;) {       // Zip64 sizes
            uint16_t id = u16(cd, x), len = u16(cd, x + 2);
            if (id == 1) {
                size_t q = x + 4;
                auto take = [&](uint64_t &v) {
                    if (v == 0xFFFFFFFF) {
                        v = uint64_t(u32(cd, q)) | uint64_t(u32(cd, q + 4)) << 32;
                        q += 8;
                    }
                };
                take(e.size);
                take(e.packed);
                take(e.header_offset);
            }
            x += 4 + len;
        }
        entries_.push_back(e);
        o += 46 + nl + xl + cl;
    }
}

ZipArchive::~ZipArchive() {
    if (f_) fclose(f_);
}

uint64_t ZipArchive::data_offset(const ZipEntry &e) const {
    Bytes h(30);
    _fseeki64(f_, int64_t(e.header_offset), SEEK_SET);
    if (fread(h.data(), 1, 30, f_) != 30 || u32(h, 0) != 0x04034b50) throw Error(e.name + ": damaged zip archive");
    return e.header_offset + 30 + u16(h, 26) + u16(h, 28);
}

void ZipArchive::extract(const ZipEntry &e, const fs::path &dst) const {
    if (e.method != 0 && e.method != 8) throw Error(e.name + ": unsupported zip compression");
    FILE *out = _wfopen(dst.wstring().c_str(), L"wb");
    if (!out) throw Error("cannot write " + path_str(dst));
    uint32_t crc = 0;
    uint64_t left = e.packed, written = 0;
    _fseeki64(f_, int64_t(data_offset(e)), SEEK_SET);
    Sink sink = [&](const uint8_t *q, size_t k) {
        crc = crc32(q, k, crc);
        written += k;
        if (fwrite(q, 1, k, out) != k) throw Error("cannot write " + path_str(dst));
    };
    try {
        if (e.method == 0) {
            std::vector<uint8_t> buf(1 << 20);
            while (left) {
                size_t k = size_t(std::min<uint64_t>(left, buf.size()));
                if (fread(buf.data(), 1, k, f_) != k) throw Error(e.name + ": damaged zip archive");
                sink(buf.data(), k);
                left -= k;
            }
        } else {
            Inflater(
                [&](uint8_t *buf, size_t want) {
                    size_t k = size_t(std::min<uint64_t>(left, want));
                    k = fread(buf, 1, k, f_);
                    left -= k;
                    return k;
                },
                sink)
                .run();
        }
    } catch (...) {
        fclose(out);
        throw;
    }
    fclose(out);
    if (written != e.size || crc != e.crc) throw Error(e.name + ": damaged zip archive (checksum)");
}

Bytes ZipArchive::read(const ZipEntry &e) const {
    Bytes packed(size_t(e.packed));
    _fseeki64(f_, int64_t(data_offset(e)), SEEK_SET);
    if (fread(packed.data(), 1, packed.size(), f_) != packed.size()) throw Error(e.name + ": damaged zip archive");
    Bytes out = e.method == 0 ? packed : inflate_raw(packed.data(), packed.size(), size_t(e.size));
    if (out.size() != e.size || crc32(out) != e.crc) throw Error(e.name + ": damaged zip archive (checksum)");
    return out;
}
