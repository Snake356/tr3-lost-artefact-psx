#include "xz.h"

#include <algorithm>
#include <array>

namespace {

[[noreturn]] void bad() { throw Error("damaged .xz data"); }

class RangeDecoder {
public:
    RangeDecoder(const uint8_t *p, const uint8_t *end) : p_(p), end_(end) {
        if (next() != 0) bad();
        for (int i = 0; i < 4; i++) code_ = code_ << 8 | next();
        if (code_ == range_) bad();
    }
    int bit(uint16_t &prob) {
        uint32_t bound = (range_ >> 11) * prob;
        int s;
        if (code_ < bound) {
            prob = uint16_t(prob + (((1u << 11) - prob) >> 5));
            range_ = bound;
            s = 0;
        } else {
            prob = uint16_t(prob - (prob >> 5));
            code_ -= bound;
            range_ -= bound;
            s = 1;
        }
        normalize();
        return s;
    }
    uint32_t direct(int n) {
        uint32_t res = 0;
        while (n--) {
            range_ >>= 1;
            code_ -= range_;
            uint32_t t = 0u - (code_ >> 31);
            code_ += range_ & t;
            normalize();
            res = (res << 1) + (t + 1);
        }
        return res;
    }
    template <size_t N> uint32_t tree(std::array<uint16_t, N> &probs, int bits) {
        uint32_t m = 1;
        for (int i = 0; i < bits; i++) m = (m << 1) + uint32_t(bit(probs[m]));
        return m - (1u << bits);
    }
    uint32_t reverse(uint16_t *probs, int bits) {
        uint32_t m = 1, sym = 0;
        for (int i = 0; i < bits; i++) {
            uint32_t b = uint32_t(bit(probs[m]));
            m = (m << 1) + b;
            sym |= b << i;
        }
        return sym;
    }

private:
    const uint8_t *p_, *end_;
    uint32_t range_ = 0xFFFFFFFF, code_ = 0;
    uint8_t next() {
        if (p_ >= end_) bad();
        return *p_++;
    }
    void normalize() {
        if (range_ < (1u << 24)) {
            range_ <<= 8;
            code_ = code_ << 8 | next();
        }
    }
};

struct LenDecoder {
    uint16_t choice = 1024, choice2 = 1024;
    std::array<std::array<uint16_t, 8>, 16> low{}, mid{};
    std::array<uint16_t, 256> high{};
    LenDecoder() {
        for (auto &a : low) a.fill(1024);
        for (auto &a : mid) a.fill(1024);
        high.fill(1024);
    }
    uint32_t decode(RangeDecoder &rc, uint32_t pos_state) {
        if (!rc.bit(choice)) return rc.tree(low[pos_state], 3);
        if (!rc.bit(choice2)) return 8 + rc.tree(mid[pos_state], 3);
        return 16 + rc.tree(high, 8);
    }
};

// The LZMA state of an LZMA2 stream; the dictionary is the whole output.
class Lzma {
public:
    void props(uint8_t d) {
        if (d >= 9 * 5 * 5) bad();
        lc_ = d % 9;
        d /= 9;
        lp_ = d % 5;
        pb_ = d / 5;
        if (lc_ + lp_ > 4) bad();
    }
    void reset() {
        lit_.assign(size_t(0x300) << (lc_ + lp_), 1024);
        auto fill = [](auto &a) { std::fill(std::begin(a), std::end(a), uint16_t(1024)); };
        fill(is_match_), fill(is_rep_), fill(is_rep_g0_), fill(is_rep_g1_), fill(is_rep_g2_), fill(is_rep0_long_);
        for (auto &a : pos_slot_) a.fill(1024);
        fill(pos_), align_.fill(1024);
        len_ = LenDecoder();
        rep_len_ = LenDecoder();
        state_ = 0;
        rep0_ = rep1_ = rep2_ = rep3_ = 0;
    }
    void chunk(Bytes &out, size_t base, const uint8_t *p, const uint8_t *end, size_t unpacked) {
        RangeDecoder rc(p, end);
        size_t stop = out.size() + unpacked;
        uint32_t pb_mask = (1u << pb_) - 1, lp_mask = (1u << lp_) - 1;
        while (out.size() < stop) {
            size_t pos = out.size() - base;
            uint32_t ps = uint32_t(pos) & pb_mask;
            if (!rc.bit(is_match_[state_ << 4 | ps])) {
                uint8_t prev = out.size() > base ? out.back() : 0;
                uint16_t *probs = &lit_[0x300 * ((((uint32_t(pos) & lp_mask) << lc_) + (prev >> (8 - lc_))))];
                uint32_t sym = 1;
                if (state_ >= 7) {
                    uint32_t match = at(out, base);
                    do {
                        uint32_t mb = (match >> 7) & 1;
                        match <<= 1;
                        uint32_t b = uint32_t(rc.bit(probs[((1 + mb) << 8) + sym]));
                        sym = sym << 1 | b;
                        if (mb != b) break;
                    } while (sym < 0x100);
                }
                while (sym < 0x100) sym = sym << 1 | uint32_t(rc.bit(probs[sym]));
                out.push_back(uint8_t(sym));
                state_ = state_ < 4 ? 0 : state_ < 10 ? state_ - 3 : state_ - 6;
                continue;
            }
            uint32_t len;
            if (!rc.bit(is_rep_[state_])) {
                rep3_ = rep2_, rep2_ = rep1_, rep1_ = rep0_;
                len = len_.decode(rc, ps);
                state_ = state_ < 7 ? 7 : 10;
                rep0_ = distance(rc, len);
                if (rep0_ == 0xFFFFFFFF) bad();        // LZMA2 has no end marker
            } else {
                if (!rc.bit(is_rep_g0_[state_])) {
                    if (!rc.bit(is_rep0_long_[state_ << 4 | ps])) {
                        state_ = state_ < 7 ? 9 : 11;
                        out.push_back(at(out, base));
                        continue;
                    }
                } else {
                    uint32_t dist;
                    if (!rc.bit(is_rep_g1_[state_])) {
                        dist = rep1_;
                    } else {
                        if (!rc.bit(is_rep_g2_[state_])) {
                            dist = rep2_;
                        } else {
                            dist = rep3_;
                            rep3_ = rep2_;
                        }
                        rep2_ = rep1_;
                    }
                    rep1_ = rep0_;
                    rep0_ = dist;
                }
                len = rep_len_.decode(rc, ps);
                state_ = state_ < 7 ? 8 : 11;
            }
            for (len += 2; len-- && out.size() < stop;) out.push_back(at(out, base));
        }
    }

private:
    uint32_t lc_ = 3, lp_ = 0, pb_ = 2, state_ = 0, rep0_ = 0, rep1_ = 0, rep2_ = 0, rep3_ = 0;
    std::vector<uint16_t> lit_;
    uint16_t is_match_[192], is_rep_[12], is_rep_g0_[12], is_rep_g1_[12], is_rep_g2_[12], is_rep0_long_[192];
    std::array<std::array<uint16_t, 64>, 4> pos_slot_{};
    uint16_t pos_[1 + 114];
    std::array<uint16_t, 16> align_{};
    LenDecoder len_, rep_len_;

    uint8_t at(const Bytes &out, size_t base) const {
        if (rep0_ >= out.size() - base) bad();
        return out[out.size() - rep0_ - 1];
    }
    uint32_t distance(RangeDecoder &rc, uint32_t len) {
        uint32_t slot = rc.tree(pos_slot_[std::min<uint32_t>(len, 3)], 6);
        if (slot < 4) return slot;
        int direct = int(slot >> 1) - 1;
        uint32_t dist = (2 | (slot & 1)) << direct;
        if (slot < 14) return dist + rc.reverse(pos_ + dist - slot, direct);
        dist += rc.direct(direct - 4) << 4;
        return dist + rc.reverse(align_.data(), 4);
    }
};

uint64_t vli(const Bytes &b, size_t &o) {
    uint64_t v = 0;
    for (int i = 0; i < 9; i++) {
        uint8_t c = u8(b, o++);
        v |= uint64_t(c & 0x7F) << (7 * i);
        if (!(c & 0x80)) return v;
    }
    bad();
}

}

Bytes xz_decompress(const Bytes &in) {
    static const uint8_t MAGIC[6] = {0xFD, '7', 'z', 'X', 'Z', 0};
    if (in.size() < 12 + 12 || !std::equal(MAGIC, MAGIC + 6, in.begin())) bad();
    int check = in[7] & 0x0F;
    size_t check_size = check == 0 ? 0 : check == 1 ? 4 : check == 4 ? 8 : check == 10 ? 32 : 0;
    if (crc32(&in[6], 2) != u32(in, 8)) bad();
    Bytes out;
    size_t o = 12;
    while (u8(in, o) != 0) {                  // blocks until the index
        size_t hsize = (size_t(in[o]) + 1) * 4;
        check_range(in, o, hsize);
        if (crc32(&in[o], hsize - 4) != u32(in, o + hsize - 4)) bad();
        size_t h = o + 1;
        uint8_t flags = in[h++];
        if (flags & 0x40) vli(in, h);
        if (flags & 0x80) vli(in, h);
        if ((flags & 3) != 0) bad();                       // one filter: LZMA2
        if (vli(in, h) != 0x21 || vli(in, h) != 1) bad();
        h++;                                               // dictionary size
        o += hsize;
        Lzma lz;
        bool need_dict = true, need_props = true;
        size_t base = out.size();
        for (;;) {
            uint8_t c = u8(in, o++);
            if (c == 0) break;
            if (c == 1 || c == 2) {                        // uncompressed chunk
                if (c == 2 && need_dict) bad();
                need_dict = false;
                size_t n = (size_t(u8(in, o)) << 8 | u8(in, o + 1)) + 1;
                o += 2;
                check_range(in, o, n);
                out.insert(out.end(), in.begin() + long(o), in.begin() + long(o + n));
                o += n;
                continue;
            }
            if (c < 0x80) bad();
            size_t unpacked = (size_t(c & 0x1F) << 16 | size_t(u8(in, o)) << 8 | u8(in, o + 1)) + 1;
            size_t packed = (size_t(u8(in, o + 2)) << 8 | u8(in, o + 3)) + 1;
            o += 4;
            int reset = (c >> 5) & 3;
            if (reset == 3) need_dict = false;
            else if (need_dict) bad();
            if (reset >= 2) {
                lz.props(u8(in, o++));
                need_props = false;
            } else if (need_props) {
                bad();
            }
            if (reset >= 1) lz.reset();
            check_range(in, o, packed);
            lz.chunk(out, base, &in[o], &in[o] + packed, unpacked);
            o += packed;
        }
        while ((o - 12) % 4) o++;                          // block padding (counted from the stream start)
        o += check_size;
    }
    return out;
}
