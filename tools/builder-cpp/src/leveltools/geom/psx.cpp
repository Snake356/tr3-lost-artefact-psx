// The animation lists of a PSX level (Lara's home), found as psx_lara_anims.py sections() finds them: the
// counts of psx_inventory.py's walk through the file, then the one place where mesh data, mesh pointers and
// animations follow each other with those counts.
#include "geom.h"

namespace geom {

namespace {

struct Reader {
    const Bytes &d;
    int64_t o = 0;

    size_t at(size_t n) const {
        if (o < 0) throw Error("read before the start of the data");
        check_range(d, size_t(o), n);
        return size_t(o);
    }
    uint32_t u8() { uint32_t v = d[at(1)]; o += 1; return v; }
    uint32_t u16() { uint32_t v = ::u16(d, at(2)); o += 2; return v; }
    int32_t i16() { int32_t v = ::s16(d, at(2)); o += 2; return v; }
    uint32_t u32() { uint32_t v = ::u32(d, at(4)); o += 4; return v; }
    int32_t i32() { int32_t v = ::s32(d, at(4)); o += 4; return v; }
    void skip(int64_t n) { o += n; }
};

void read_room(Reader &r) {
    r.at(16);
    r.skip(16);
    int64_t ndw = r.u32();
    int64_t start = r.o;
    uint32_t n_roomlets = r.u32();
    std::vector<uint32_t> offs;
    for (uint32_t i = 0; i < n_roomlets; i++) offs.push_back(r.u32());
    for (uint32_t off : offs) {
        r.o = start + off;
        r.skip(6);                      // bbox
        uint32_t v = r.u8();
        uint32_t t = r.u8();
        r.skip(int64_t(v) * 4 + int64_t(t) * 4);
        bool more = true;
        while (more) {
            uint32_t tex = r.u32();
            r.skip(12);
            for (int k = 0; k < 3; k++)
                if (((tex >> (10 * k)) & 0x3ff) == 0x3ff) {
                    more = false;
                    break;
                }
        }
    }
    r.o = start + ndw * 2;
    uint32_t npo = r.u16();
    r.skip(int64_t(npo) * 32);
    uint32_t nz = r.u16();
    uint32_t nx = r.u16();
    r.skip(int64_t(nz) * nx * 8);
    r.i16();
    r.i16();
    uint32_t nl = r.u16();
    r.skip(int64_t(nl) * 24);
    uint32_t ns = r.u16();
    r.skip(int64_t(ns) * 20);
    r.i16();
    r.u16();
    r.u8();
    r.u8();
    r.skip(1);
}

struct Counts {
    uint32_t meshdata, meshptr, anims;
    int64_t entities;      // offset of the entity list (behind its count)
};

// psx_inventory.py inventory(): the walk must reach the end of the sound details for the status "ok".
Counts inventory(const Bytes &d) {
    Reader r{d};
    Counts z{};
    try {
        r.i32();
        uint32_t n = r.u32();
        r.skip(int64_t(n) * 4);
        if (n) {                        // without samples there is no size field
            uint32_t size = r.u32();
            r.skip(size);
        }
        for (int i = 0; i < 13; i++) {
            int32_t s = r.i32();
            if (s != 0) {
                r.skip(s);
                int32_t s2 = r.i32();
                r.skip(s2);
            }
        }
        uint32_t nr = r.u16();
        for (uint32_t i = 0; i < nr; i++) read_room(r);
        r.skip(int64_t(r.u32()) * 2);   // floordata
        r.skip(729 * 2);
        r.skip(r.u32());                // outdoor room table
        r.skip(int64_t(r.i32()) * 8);   // bounding boxes
        z.meshdata = r.u32();
        r.skip(int64_t(z.meshdata) * 2);
        z.meshptr = r.u32();
        r.skip(int64_t(z.meshptr) * 4);
        z.anims = r.u32();
        r.skip(int64_t(z.anims) * 32);
        r.skip(int64_t(r.u32()) * 6);
        r.skip(int64_t(r.u32()) * 8);
        r.skip(int64_t(r.u32()) * 2);
        r.skip(int64_t(r.u32()) * 4);
        r.skip(int64_t(r.u32()) * 2);
        r.skip(int64_t(r.u32()) * 20);
        r.skip(int64_t(r.u32()) * 32);
        r.skip(int64_t(r.u32()) * 32768);
        uint32_t ncl = r.u32();
        if (ncl & 0xffff0000) {         // Japan: two bytes more in front
            r.o -= 2;
            ncl = r.u32();
        }
        r.skip(int64_t(ncl) * 2 * 32);
        r.skip(int64_t(r.u32()) * 16);  // object textures
        r.skip(int64_t(r.u32()) * 16);  // sprite textures
        r.skip(int64_t(r.u32()) * 8);   // sprite sequences
        r.skip(int64_t(r.u32()) * 16);  // cameras
        r.skip(int64_t(r.u32()) * 16);  // sound sources
        uint32_t nbx = r.u32();
        r.skip(int64_t(nbx) * 8);
        r.skip(int64_t(r.u32()) * 2);   // overlaps
        r.skip(int64_t(nbx) * 20);
        r.skip(int64_t(r.u32()) * 2);   // animated textures
        uint32_t nen = r.u32();
        z.entities = r.o;
        r.skip(int64_t(nen) * 24);
        r.u32();                        // horizon colour
        r.skip(int64_t(r.u32()) * 48);  // room textures
        r.skip(370 * 2);
        uint32_t nsd = r.u32();
        r.skip(int64_t(nsd) * 8);
    } catch (const std::exception &e) {
        throw Error("ERROR @" + std::to_string(r.o) + ": " + e.what());
    }
    return z;
}

}  // namespace

PsxSections psx_sections(const Bytes &d) {
    Counts z = inventory(d);
    uint64_t nmd = z.meshdata, nmp = z.meshptr, na = z.anims;
    std::vector<size_t> hits;
    for (size_t o = 0; o + 12 < d.size(); o++) {
        if (u32(d, o) != nmd) continue;
        if (!(o + 8 + nmd * 2 + nmp * 4 < d.size())) continue;
        if (u32(d, size_t(o + 4 + nmd * 2)) == nmp && u32(d, size_t(o + 8 + nmd * 2 + nmp * 4)) == na) hits.push_back(o);
    }
    if (hits.size() != 1) throw Error(std::to_string(hits.size()) + " places of the animation lists found");
    size_t o = size_t(hits[0] + 8 + nmd * 2 + nmp * 4);
    PsxSections s{};
    auto next = [&](size_t &field, size_t size) {
        field = o;
        o += 4 + size * size_t(u32(d, o));
    };
    next(s.anims, 32);
    next(s.states, 6);
    next(s.dispatch, 8);
    next(s.commands, 2);
    next(s.trees, 4);
    next(s.frames, 2);
    next(s.moveables, PSX_MODEL);
    s.items = size_t(z.entities - 4);
    return s;
}

}  // namespace geom
