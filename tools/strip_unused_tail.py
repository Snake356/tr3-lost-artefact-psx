"""Removes data from a TR3 PC level that nothing uses, as level editors leave it behind (e.g. after
importing a model): meshes no moveable/static uses, animations after the last used one, frames after
the last used frame and object textures after the last used one. Everything that is used keeps its
index, so items, triggers, moveables and textures stay unchanged.

Only unused TRAILING animations/frames/object textures are removed; unused meshes are removed
anywhere (the mesh pointers are renumbered only if all unused pointers are at the end, otherwise
the script refuses). The file is checked again after writing.

Usage: strip_unused_tail.py in.tr2 out.tr2
"""
import struct
import sys


def u16(b, o): return struct.unpack_from("<H", b, o)[0]
def i16(b, o): return struct.unpack_from("<h", b, o)[0]
def u32(b, o): return struct.unpack_from("<I", b, o)[0]


def layout(b):
    """Offsets of the count fields of the arrays between the rooms and the items."""
    L = {}
    p = 4 + 768 + 1024
    p += 4 + u32(b, p) * 65536 * 3 + 4
    L["rooms"] = p
    nr = u16(b, p); p += 2
    faces = []  # (offset of texture word) of all textured room faces
    for _ in range(nr):
        p += 16
        ndw = u32(b, p); p += 4
        d = p
        nv = u16(b, d); d += 2 + nv * 12
        nq = u16(b, d); d += 2
        faces += [d + k * 10 + 8 for k in range(nq)]; d += nq * 10
        nt = u16(b, d); d += 2
        faces += [d + k * 8 + 6 for k in range(nt)]
        p += ndw * 2
        p += 2 + u16(b, p) * 32
        zs, xs = u16(b, p), u16(b, p + 2); p += 4 + zs * xs * 8
        p += 4
        p += 2 + u16(b, p) * 24
        p += 2 + u16(b, p) * 20
        p += 7
    L["room_faces"] = faces
    for name, size in (("floordata", 2), ("meshdata", 2), ("meshptr", 4), ("anims", 32), ("statech", 6),
                       ("dispatch", 8), ("animcmd", 2), ("meshtree", 4), ("frames", 2), ("moveables", 18),
                       ("statics", 32), ("sprtex", 16), ("sprseq", 8), ("cameras", 16), ("soundsrc", 16)):
        L[name] = (p, u32(b, p), size); p += 4 + u32(b, p) * size
    nbox = u32(b, p); p += 4 + nbox * 8
    p += 4 + u32(b, p) * 2          # overlaps
    p += nbox * 20                  # zones
    L["animtex"] = (p, u32(b, p), 2); p += 4 + u32(b, p) * 2
    L["objtex"] = (p, u32(b, p), 20); p += 4 + u32(b, p) * 20
    L["items"] = (p, u32(b, p), 24)
    return L


def mesh_info(b, off):
    """Size in bytes and the object texture indices used by the mesh at off."""
    p = off + 10
    nv = i16(b, p); p += 2 + nv * 6
    nn = i16(b, p); p += 2 + (nn * 6 if nn > 0 else -nn * 2)
    tex = []
    n = i16(b, p); tex += [u16(b, p + 2 + k * 10 + 8) & 0x7FFF for k in range(n)]; p += 2 + n * 10
    n = i16(b, p); tex += [u16(b, p + 2 + k * 8 + 6) & 0x7FFF for k in range(n)]; p += 2 + n * 8
    n = i16(b, p); p += 2 + n * 10
    n = i16(b, p); p += 2 + n * 8
    return p - off, tex


def analyse(b):
    L = layout(b)
    md = L["meshdata"][0] + 4
    pp, nptr, _ = L["meshptr"]
    ptr = [u32(b, pp + 4 + i * 4) for i in range(nptr)]
    mv = [struct.unpack_from("<IHHIIH", b, L["moveables"][0] + 4 + i * 18) for i in range(L["moveables"][1])]
    st = [struct.unpack_from("<IH", b, L["statics"][0] + 4 + i * 32) for i in range(L["statics"][1])]
    used_ptr = set()
    for oid, nm, sm, mt, fo, an in mv:
        used_ptr |= set(range(sm, sm + nm))
    used_ptr |= {mi for sid, mi in st}
    tex = [u16(b, o) & 0x7FFF for o in L["room_faces"]]
    for i in used_ptr:
        tex += mesh_info(b, md + ptr[i])[1]
    at = L["animtex"]
    words = [u16(b, at[0] + 4 + k * 2) for k in range(at[1])]
    k = 1
    for _ in range(words[0] if words else 0):
        n = words[k] + 1
        tex += words[k + 1:k + 1 + n]; k += 1 + n
    ap, nanim, _ = L["anims"]
    anims = [struct.unpack_from("<IBBHiiHHHHHHHH", b, ap + 4 + i * 32) for i in range(nanim)]
    # animations reachable from the moveables (next animation, state change dispatches); an
    # animation between two used ones always stays, only the unreachable tail is dropped
    sp = L["statech"][0] + 4
    dp = L["dispatch"][0] + 4
    starts = [an for (oid, nm, sm, mt, fo, an) in mv if an != 0xFFFF]
    used_anim, todo = set(), list(starts)
    while todo:
        a = todo.pop()
        if a in used_anim or a >= nanim:
            continue
        used_anim.add(a)
        todo.append(anims[a][8])
        for s in range(anims[a][10]):
            _, nds, dofs = struct.unpack_from("<HHH", b, sp + (anims[a][11] + s) * 6)
            todo += [u16(b, dp + (dofs + k) * 8 + 4) for k in range(nds)]
    last_anim = max(used_anim | set(starts))
    frames_end = max([a[0] for a in anims[:last_anim + 1]] + [fo for (_, _, _, _, fo, _) in mv])
    return L, ptr, used_ptr, max(tex), last_anim, frames_end


def main(src, dst):
    b = bytearray(open(src, "rb").read())
    L, ptr, used_ptr, last_tex, last_anim, frames_end = analyse(b)
    nptr = len(ptr)
    unused = sorted(set(range(nptr)) - used_ptr)
    if unused and unused != list(range(nptr - len(unused), nptr)):
        sys.exit(f"unused mesh pointers are not all at the end: {unused[:20]} - not changed")
    keep = nptr - len(unused)
    md = L["meshdata"][0] + 4
    # mesh data: keep the used meshes in their order, drop the rest
    offs = sorted({ptr[i] for i in range(keep)})
    new_md, newoff = bytearray(), {}
    for o in offs:
        size = mesh_info(b, md + o)[0]
        newoff[o] = len(new_md)
        new_md += b[md + o:md + o + size]
        if len(new_md) % 4:
            new_md += bytes(4 - len(new_md) % 4)
    nanim = L["anims"][1]
    nframes = L["frames"][1]
    ntex = L["objtex"][1]
    # the frames of the last used animation run up to the next animation's frames (or the end)
    ap = L["anims"][0] + 4
    later = [u32(b, ap + i * 32) for i in range(last_anim + 1, nanim)]
    frames_keep = min([nframes * 2] + [f for f in later if f > frames_end]) // 2
    print(f"meshes {nptr} -> {keep}, mesh data {L['meshdata'][1]} -> {len(new_md) // 2} words, "
          f"anims {nanim} -> {last_anim + 1}, frames {nframes} -> {frames_keep}, "
          f"object textures {ntex} -> {last_tex + 1}")

    out = bytearray(b[:L["meshdata"][0]])
    out += struct.pack("<I", len(new_md) // 2) + new_md
    out += struct.pack("<I", keep) + b"".join(struct.pack("<I", newoff[ptr[i]]) for i in range(keep))
    out += struct.pack("<I", last_anim + 1) + b[ap:ap + (last_anim + 1) * 32]
    p = L["statech"][0]
    out += b[p:L["frames"][0]]
    out += struct.pack("<I", frames_keep) + b[L["frames"][0] + 4:L["frames"][0] + 4 + frames_keep * 2]
    out += b[L["moveables"][0]:L["objtex"][0]]
    op = L["objtex"][0] + 4
    out += struct.pack("<I", last_tex + 1) + b[op:op + (last_tex + 1) * 20]
    out += b[L["items"][0]:]
    open(dst, "wb").write(out)

    # check: everything used is still there and unchanged
    b2 = bytes(out)
    L2, ptr2, used2, tex2, anim2, fe2 = analyse(b2)
    md2 = L2["meshdata"][0] + 4
    for i in sorted(used_ptr):
        s = mesh_info(b, md + ptr[i])[0]
        assert b[md + ptr[i]:md + ptr[i] + s] == b2[md2 + ptr2[i]:md2 + ptr2[i] + s], f"mesh {i} changed"
    assert b2[L2["moveables"][0]:L2["objtex"][0]] == b[L["moveables"][0]:L["objtex"][0]]
    assert b2[L2["items"][0]:] == b[L["items"][0]:]
    assert (tex2, anim2, fe2) == (last_tex, last_anim, frames_end)
    print(f"written {dst} ({len(b)} -> {len(b2)} bytes), check ok")


if __name__ == "__main__":
    if len(sys.argv) != 3:
        sys.exit(__doc__)
    main(sys.argv[1], sys.argv[2])
