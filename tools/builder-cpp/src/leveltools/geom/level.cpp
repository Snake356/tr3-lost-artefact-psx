// The TR3 PC level layout of strip_unused_tail.py and drop_object.py, and Python's number parsing.
#include <algorithm>
#include <cctype>
#include <cerrno>
#include <cstdlib>
#include <format>
#include <map>

#include "geom.h"

namespace geom {

namespace {

// b[a:e] with Python's clamping
Bytes pyslice(const Bytes &b, size_t a, size_t e) {
    a = std::min(a, b.size());
    e = std::min(std::max(e, a), b.size());
    return Bytes(b.begin() + ptrdiff_t(a), b.begin() + ptrdiff_t(e));
}

void put_u32(Bytes &out, uint32_t v) { Writer{out}.u32(v); }

}  // namespace

// ---- Python semantics -------------------------------------------------------------------------------------

int64_t parse_int(const std::string &text, int base) {
    std::string s = trim(text);
    auto bad = [&]() { return Error("invalid literal for int() with base " + std::to_string(base) + ": '" + text + "'"); };
    size_t i = 0;
    bool neg = false;
    if (i < s.size() && (s[i] == '+' || s[i] == '-')) neg = s[i++] == '-';
    int b = base;
    if (base == 0 || base == 16 || base == 8 || base == 2) {
        if (i + 1 < s.size() && s[i] == '0') {
            char c = char(std::tolower(static_cast<unsigned char>(s[i + 1])));
            int pb = c == 'x' ? 16 : c == 'o' ? 8 : c == 'b' ? 2 : 0;
            if (pb && (base == 0 || base == pb)) {
                b = pb;
                i += 2;
                if (i < s.size() && s[i] == '_') i++;
            }
        }
        if (b == 0) b = 10;
    }
    std::string digits;
    bool last_us = true;
    for (; i < s.size(); i++) {
        if (s[i] == '_') {
            if (last_us) throw bad();
            last_us = true;
            continue;
        }
        digits += s[i];
        last_us = false;
    }
    if (digits.empty() || last_us) throw bad();
    int64_t v = 0;
    for (char c : digits) {
        int d = std::isdigit(static_cast<unsigned char>(c)) ? c - '0'
                : std::isalpha(static_cast<unsigned char>(c)) ? std::tolower(static_cast<unsigned char>(c)) - 'a' + 10
                                                               : 99;
        if (d >= b) throw bad();
        v = v * b + d;
    }
    // base 0: a decimal number must not have leading zeros (except zero itself)
    if (base == 0 && b == 10 && digits.size() > 1 && digits[0] == '0' && v != 0) throw bad();
    return neg ? -v : v;
}

double parse_float(const std::string &text) {
    std::string s = trim(text);
    std::string t;
    for (char c : s)
        if (c != '_') t += c;
    if (t.empty()) throw Error("could not convert string to float: '" + text + "'");
    char *end = nullptr;
    errno = 0;
    double v = std::strtod(t.c_str(), &end);
    if (end != t.c_str() + t.size()) throw Error("could not convert string to float: '" + text + "'");
    return v;
}

std::string basename(const std::string &path) {
    size_t p = path.find_last_of("/\\");
    std::string s = p == std::string::npos ? path : path.substr(p + 1);
    if (s.size() >= 2 && s[1] == ':' && p == std::string::npos) s = s.substr(2);
    return s;
}

size_t count_nonoverlapping(const Bytes &hay, const Bytes &needle) {
    size_t n = 0;
    for (size_t p = find_bytes(hay, needle); p != std::string::npos; p = find_bytes(hay, needle, p + needle.size())) n++;
    return n;
}

// ---- strip_unused_tail.py -----------------------------------------------------------------------------------

Layout layout(const Bytes &b) {
    Layout L;
    size_t p = 4 + 768 + 1024;
    p += 4 + size_t(u32(b, p)) * 65536 * 3 + 4;
    L.rooms = p;
    unsigned nr = u16(b, p);
    p += 2;
    for (unsigned r = 0; r < nr; r++) {
        p += 16;
        size_t ndw = u32(b, p);
        p += 4;
        size_t d = p;
        size_t nv = u16(b, d);
        d += 2 + nv * 12;
        size_t nq = u16(b, d);
        d += 2;
        for (size_t k = 0; k < nq; k++) L.room_faces.push_back(d + k * 10 + 8);
        d += nq * 10;
        size_t nt = u16(b, d);
        d += 2;
        for (size_t k = 0; k < nt; k++) L.room_faces.push_back(d + k * 8 + 6);
        p += ndw * 2;
        p += 2 + size_t(u16(b, p)) * 32;
        size_t zs = u16(b, p), xs = u16(b, p + 2);
        p += 4 + zs * xs * 8;
        p += 4;
        p += 2 + size_t(u16(b, p)) * 24;
        p += 2 + size_t(u16(b, p)) * 20;
        p += 7;
    }
    auto next = [&](Section &s, size_t size) {
        s = Section{p, u32(b, p), size};
        p += 4 + size_t(s.count) * size;
    };
    next(L.floordata, 2);
    next(L.meshdata, 2);
    next(L.meshptr, 4);
    next(L.anims, 32);
    next(L.statech, 6);
    next(L.dispatch, 8);
    next(L.animcmd, 2);
    next(L.meshtree, 4);
    next(L.frames, 2);
    next(L.moveables, 18);
    next(L.statics, 32);
    next(L.sprtex, 16);
    next(L.sprseq, 8);
    next(L.cameras, 16);
    next(L.soundsrc, 16);
    size_t nbox = u32(b, p);
    p += 4 + nbox * 8;
    p += 4 + size_t(u32(b, p)) * 2;     // overlaps
    p += nbox * 20;                     // zones
    next(L.animtex, 2);
    next(L.objtex, 20);
    L.items = Section{p, u32(b, p), 24};
    return L;
}

MeshInfo mesh_info(const Bytes &b, size_t off) {
    MeshInfo m;
    size_t p = off + 10;
    int64_t nv = s16(b, p);
    p = add(p, 2 + nv * 6);
    int64_t nn = s16(b, p);
    p = add(p, 2 + (nn > 0 ? nn * 6 : -nn * 2));
    int64_t n = s16(b, p);
    for (int64_t k = 0; k < n; k++) m.tex.push_back(u16(b, p + 2 + size_t(k) * 10 + 8) & 0x7FFF);
    p = add(p, 2 + n * 10);
    n = s16(b, p);
    for (int64_t k = 0; k < n; k++) m.tex.push_back(u16(b, p + 2 + size_t(k) * 8 + 6) & 0x7FFF);
    p = add(p, 2 + n * 8);
    n = s16(b, p);
    p = add(p, 2 + n * 10);
    n = s16(b, p);
    p = add(p, 2 + n * 8);
    m.size = p - off;
    return m;
}

std::vector<Moveable> moveables(const Bytes &b, const Layout &L) {
    std::vector<Moveable> mv;
    for (size_t i = 0; i < L.moveables.count; i++) {
        size_t o = L.moveables.data() + i * 18;
        mv.push_back({u32(b, o), u16(b, o + 4), u16(b, o + 6), u32(b, o + 8), u32(b, o + 12), u16(b, o + 16)});
    }
    return mv;
}

Analysis analyse(const Bytes &b) {
    Analysis A;
    A.L = layout(b);
    const Layout &L = A.L;
    size_t md = L.meshdata.data();
    for (size_t i = 0; i < L.meshptr.count; i++) A.ptr.push_back(u32(b, L.meshptr.data() + i * 4));
    std::vector<Moveable> mv = moveables(b, L);
    for (auto &m : mv)
        for (size_t i = m.start; i < size_t(m.start) + m.nmeshes; i++) A.used_ptr.insert(i);
    for (size_t i = 0; i < L.statics.count; i++) A.used_ptr.insert(u16(b, L.statics.data() + i * 32 + 4));

    bool any_tex = false;
    int max_tex = 0;
    auto tex = [&](int t) {
        if (!any_tex || t > max_tex) max_tex = t;
        any_tex = true;
    };
    for (size_t o : L.room_faces) tex(u16(b, o) & 0x7FFF);
    for (size_t i : A.used_ptr)
        for (int t : mesh_info(b, md + A.ptr.at(i)).tex) tex(t);
    std::vector<int> words;
    for (size_t k = 0; k < L.animtex.count; k++) words.push_back(u16(b, L.animtex.data() + k * 2));
    size_t k = 1;
    int groups = words.empty() ? 0 : words[0];
    for (int g = 0; g < groups; g++) {
        size_t n = size_t(words.at(k)) + 1;
        for (size_t j = k + 1; j < std::min(k + 1 + n, words.size()); j++) tex(words[j]);
        k += 1 + n;
    }
    if (!any_tex) throw Error("max() arg is an empty sequence");
    A.last_tex = max_tex;

    size_t ap = L.anims.data(), nanim = L.anims.count;
    size_t sp = L.statech.data(), dp = L.dispatch.data();
    std::vector<int64_t> starts;
    for (auto &m : mv)
        if (m.anim != 0xFFFF) starts.push_back(m.anim);
    std::set<int64_t> used_anim;
    std::vector<int64_t> todo(starts);
    while (!todo.empty()) {
        int64_t a = todo.back();
        todo.pop_back();
        if (used_anim.count(a) || a >= int64_t(nanim)) continue;
        used_anim.insert(a);
        size_t rec = ap + size_t(a) * 32;
        todo.push_back(u16(b, rec + 20));
        unsigned nsc = u16(b, rec + 24), sco = u16(b, rec + 26);
        for (unsigned s = 0; s < nsc; s++) {
            size_t sc = sp + (size_t(sco) + s) * 6;
            unsigned nds = u16(b, sc + 2), dofs = u16(b, sc + 4);
            for (unsigned j = 0; j < nds; j++) todo.push_back(u16(b, dp + (size_t(dofs) + j) * 8 + 4));
        }
    }
    std::set<int64_t> all(used_anim);
    all.insert(starts.begin(), starts.end());
    if (all.empty()) throw Error("max() arg is an empty sequence");
    A.last_anim = *all.rbegin();
    bool any = false;
    int64_t fe = 0;
    for (size_t i = 0; i < std::min(size_t(A.last_anim + 1), nanim); i++) {
        int64_t f = u32(b, ap + i * 32);
        if (!any || f > fe) fe = f;
        any = true;
    }
    for (auto &m : mv) {
        if (!any || int64_t(m.frame) > fe) fe = m.frame;
        any = true;
    }
    if (!any) throw Error("max() arg is an empty sequence");
    A.frames_end = fe;
    return A;
}

void strip_unused_tail(ToolContext &ctx, const std::string &src, const std::string &dst) {
    Bytes b = read_file(ctx.path(src));
    Analysis A = analyse(b);
    const Layout &L = A.L;
    size_t nptr = A.ptr.size();
    std::vector<size_t> unused;
    for (size_t i = 0; i < nptr; i++)
        if (!A.used_ptr.count(i)) unused.push_back(i);
    bool tail = true;
    for (size_t j = 0; j < unused.size(); j++)
        if (unused[j] != nptr - unused.size() + j) tail = false;
    if (!unused.empty() && !tail) {
        std::string list;
        for (size_t j = 0; j < std::min<size_t>(20, unused.size()); j++) list += (j ? ", " : "") + std::to_string(unused[j]);
        throw Error("unused mesh pointers are not all at the end: [" + list + "] - not changed");
    }
    size_t keep = nptr - unused.size();
    size_t md = L.meshdata.data();
    std::set<uint32_t> offs;
    for (size_t i = 0; i < keep; i++) offs.insert(A.ptr[i]);
    Bytes new_md;
    std::map<uint32_t, uint32_t> newoff;
    for (uint32_t o : offs) {
        size_t size = mesh_info(b, md + o).size;
        newoff[o] = uint32_t(new_md.size());
        Writer{new_md}.bytes(pyslice(b, md + o, md + o + size));
        if (new_md.size() % 4) new_md.resize(new_md.size() + 4 - new_md.size() % 4);
    }
    size_t nanim = L.anims.count, nframes = L.frames.count, ntex = L.objtex.count;
    size_t ap = L.anims.data();
    int64_t fk = int64_t(nframes) * 2;
    for (size_t i = size_t(A.last_anim + 1); i < nanim; i++) {
        int64_t f = u32(b, ap + i * 32);
        if (f > A.frames_end) fk = std::min(fk, f);
    }
    size_t frames_keep = size_t(fk / 2);
    ctx.print(std::format("meshes {} -> {}, mesh data {} -> {} words, anims {} -> {}, frames {} -> {}, "
                          "object textures {} -> {}",
                          nptr, keep, L.meshdata.count, new_md.size() / 2, nanim, A.last_anim + 1, nframes,
                          frames_keep, ntex, A.last_tex + 1));

    Bytes out = pyslice(b, 0, L.meshdata.off);
    put_u32(out, uint32_t(new_md.size() / 2));
    Writer{out}.bytes(new_md);
    put_u32(out, uint32_t(keep));
    for (size_t i = 0; i < keep; i++) put_u32(out, newoff[A.ptr[i]]);
    put_u32(out, uint32_t(A.last_anim + 1));
    Writer{out}.bytes(pyslice(b, ap, ap + size_t(A.last_anim + 1) * 32));
    Writer{out}.bytes(pyslice(b, L.statech.off, L.frames.off));
    put_u32(out, uint32_t(frames_keep));
    Writer{out}.bytes(pyslice(b, L.frames.data(), L.frames.data() + frames_keep * 2));
    Writer{out}.bytes(pyslice(b, L.moveables.off, L.objtex.off));
    size_t op = L.objtex.data();
    put_u32(out, uint32_t(A.last_tex + 1));
    Writer{out}.bytes(pyslice(b, op, op + size_t(A.last_tex + 1) * 20));
    Writer{out}.bytes(pyslice(b, L.items.off, b.size()));
    write_file(ctx.path(dst), out);

    Analysis A2 = analyse(out);
    size_t md2 = A2.L.meshdata.data();
    for (size_t i : A.used_ptr) {
        size_t s = mesh_info(b, md + A.ptr[i]).size;
        if (pyslice(b, md + A.ptr[i], md + A.ptr[i] + s) != pyslice(out, md2 + A2.ptr.at(i), md2 + A2.ptr.at(i) + s))
            throw Error("mesh " + std::to_string(i) + " changed");
    }
    if (pyslice(out, A2.L.moveables.off, A2.L.objtex.off) != pyslice(b, L.moveables.off, L.objtex.off) ||
        pyslice(out, A2.L.items.off, out.size()) != pyslice(b, L.items.off, b.size()) ||
        A2.last_tex != A.last_tex || A2.last_anim != A.last_anim || A2.frames_end != A.frames_end)
        throw Error("check failed");
    ctx.print(std::format("written {} ({} -> {} bytes), check ok", dst, b.size(), out.size()));
}

// ---- drop_object.py ------------------------------------------------------------------------------------------

Sections sections(const Bytes &d) {
    Sections s{};
    size_t o = 4 + 768 + 1024;
    size_t n = u32(d, o);
    o += 4 + n * 65536 + n * 131072 + 4;
    unsigned rooms = u16(d, o);
    o += 2;
    for (unsigned r = 0; r < rooms; r++) {
        o += 16;
        o += 4 + 2 * size_t(u32(d, o));
        o += 2 + 32 * size_t(u16(d, o));
        size_t zs = u16(d, o), xs = u16(d, o + 2);
        o += 4 + 8 * zs * xs + 4;
        o += 2 + 24 * size_t(u16(d, o));
        o += 2 + 20 * size_t(u16(d, o));
        o += 7;
    }
    auto next = [&](size_t &field, size_t size) {
        field = o;
        o += 4 + size * size_t(u32(d, o));
    };
    next(s.floor, 2);
    next(s.mesh, 2);
    next(s.meshptr, 4);
    next(s.anims, 32);
    next(s.states, 6);
    next(s.dispatch, 8);
    next(s.commands, 2);
    next(s.trees, 4);
    next(s.frames, 2);
    next(s.moveables, 18);
    next(s.statics, 32);
    next(s.spritetex, 16);
    next(s.spriteseq, 8);
    next(s.cameras, 16);
    next(s.sources, 16);
    size_t boxes = u32(d, o);
    o += 4 + 8 * boxes;
    o += 4 + 2 * size_t(u32(d, o));     // overlaps
    o += 20 * boxes;                    // zones
    o += 4 + 2 * size_t(u32(d, o));     // animated textures
    o += 4 + 20 * size_t(u32(d, o));    // object textures
    s.items = o;
    return s;
}

std::optional<Bytes> drop(const Bytes &d, uint32_t oid, std::string &what) {
    Sections s = sections(d);
    auto count = [&](size_t field) { return size_t(u32(d, field)); };
    struct Mov {
        uint32_t id;
        size_t n, start;
    };
    std::vector<Mov> movs;
    for (size_t i = 0; i < count(s.moveables); i++) {
        size_t o = s.moveables + 4 + 18 * i;
        movs.push_back({u32(d, o), u16(d, o + 4), u16(d, o + 6)});
    }
    const Mov *mine = nullptr;
    for (auto &m : movs)
        if (m.id == oid) {
            mine = &m;
            break;
        }
    if (!mine) {
        what = "not in the level";
        return std::nullopt;
    }
    for (size_t i = 0; i < count(s.items); i++)
        if (int64_t(s16(d, s.items + 4 + 24 * i)) == int64_t(oid)) {
            what = "placed in the level";
            return std::nullopt;
        }
    size_t nmesh = mine->n, start = mine->start;
    std::vector<uint32_t> ptrs;
    for (size_t i = 0; i < count(s.meshptr); i++) ptrs.push_back(u32(d, s.meshptr + 4 + 4 * i));
    auto own = [&](size_t i) { return i >= start && i < start + nmesh; };
    std::set<size_t> used_elsewhere;
    for (auto &m : movs)
        if (m.id != oid)
            for (size_t i = m.start; i < m.start + m.n; i++) used_elsewhere.insert(i);
    for (size_t i = 0; i < count(s.statics); i++) used_elsewhere.insert(u16(d, s.statics + 4 + 32 * i + 4));
    std::set<uint32_t> other, cut;
    for (size_t i = 0; i < ptrs.size(); i++)
        if (!own(i) || used_elsewhere.count(i)) other.insert(ptrs[i]);
    for (size_t i = start; i < start + nmesh; i++) {
        uint32_t p = ptrs.at(i);
        if (!other.count(p) && p != 0) cut.insert(p);
    }
    if (cut.empty()) {
        what = "no meshes of its own";
        return std::nullopt;
    }
    std::set<int64_t> endset(ptrs.begin(), ptrs.end());
    endset.insert(2 * int64_t(count(s.mesh)));
    std::vector<int64_t> ends(endset.begin(), endset.end());
    std::vector<std::pair<int64_t, int64_t>> ranges;
    for (uint32_t p : cut) {
        auto it = std::find(ends.begin(), ends.end(), int64_t(p));
        if (it + 1 == ends.end()) throw Error("list index out of range");
        ranges.push_back({p, *(it + 1)});
    }
    int64_t removed = 0;
    for (auto &[a, e] : ranges) removed += e - a;
    auto shift = [&](int64_t p) {
        int64_t v = p;
        for (auto &[a, e] : ranges)
            if (e <= p) v -= e - a;
        return v;
    };
    std::vector<uint32_t> new_ptrs;
    for (size_t i = 0; i < ptrs.size(); i++) new_ptrs.push_back(own(i) ? 0 : uint32_t(shift(ptrs[i])));
    size_t md = s.mesh + 4;
    Bytes data = pyslice(d, md, md + 2 * count(s.mesh));
    Bytes kept;
    int64_t from = 0;
    for (auto &[a, e] : ranges) {
        Writer{kept}.bytes(pyslice(data, size_t(from), size_t(a)));
        from = e;
    }
    Writer{kept}.bytes(pyslice(data, size_t(from), data.size()));
    Bytes out = pyslice(d, 0, s.mesh);
    put_u32(out, uint32_t(kept.size() / 2));
    Writer{out}.bytes(kept);
    put_u32(out, uint32_t(new_ptrs.size()));
    for (uint32_t p : new_ptrs) put_u32(out, p);
    Writer{out}.bytes(pyslice(d, s.meshptr + 4 + 4 * ptrs.size(), d.size()));
    what = std::format("{} meshes, {} bytes removed", ranges.size(), removed);
    return out;
}

}  // namespace geom
