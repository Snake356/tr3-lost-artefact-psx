"""Marks the rooms with a mist layer (additive faces lying flat on a solid floor) with room flag 0x4000 and their
strength in bits 0x1800 (both unused in every Lost Artefact level; the PS1 room keeps the flags at +0x4E), so the
executable's mist sorting (mistsort.ips) can push those rooms' semi-transparent faces forward, without water
surfaces (additive too, but over a water room, also one that only floods with the flipmap) showing through
jetties. No room holds both. Strength 128 unless STRENGTH says otherwise.
Usage: nebel_raeume.py level.tr2 [--list]"""
import os
import struct
import sys

import split_room as S
import strip_unused_tail as T

MIST, STRENGTH_BITS = 0x4000, 0x1800
LEVELS = {128: 0, 64: 1, 32: 2, 16: 3}      # strength -> the value in bits 0x1800 (the PS1 bias: 128 >> it)


def room_flags(flags, strength):
    """The room flags for a mist strength (0 = no bias)."""
    flags &= ~(MIST | STRENGTH_BITS)
    return flags if strength == 0 else flags | MIST | LEVELS[strength] << 11


# per level: room -> strength (0 = the game's order). The green pools of Madhouse and the green corridors of
# Reunion: Lara wades through the layer and it also glows on the walls; 0 leaves steps at the layer's edges
# (floor and walls drawn over it), 16 closes them without drawing it over Lara
STRENGTH = {"scotland.tr2": dict.fromkeys((44, 55, 56, 59, 64, 65, 84, 97, 105, 109, 123), 32),   # 128: the mist over the walls of the layer below
            "zoo.tr2": dict.fromkeys((51, 124, 130, 133), 16),
            "slinc.tr2": dict.fromkeys((3, 5, 7, 30, 34, 36, 52, 56, 61, 66, 68), 16),
            "willsden.tr2": {**dict.fromkeys((6, 25, 26, 28, 35, 36, 37, 56, 62, 81), 64),   # 0: gaps, 128: the mist over the walls
                             86: 16,        # 0: gaps, 32 and more: the mist over the wall edge to room 87
                             90: 16},       # 128: the mist over the pillar's foot
            "undersea.tr2": {19: 0}}        # water outside, where the divers jump in (the room below is dry)


def classify(b):
    """(mist rooms, water rooms): rooms with additive flat faces on a floor / over a water room."""
    o, n, _ = T.layout(b)["objtex"]
    attr = [struct.unpack_from("<H", b, o + 4 + 20 * i)[0] for i in range(n)]
    _, rooms, _, _ = S.parse_rooms(b)
    mist, water = set(), set()
    for ri, r in enumerate(rooms):
        for f, k in [(q, 4) for q in r.quads] + [(t, 3) for t in r.tris]:
            if attr[f[k] & 0x7FFF] != 2:
                continue
            vs = [r.verts[i] for i in f[:k]]
            ys = [v[1] for v in vs]
            if max(ys) - min(ys) >= 128:
                continue
            xi, zi = sum(v[0] for v in vs) // k // 1024, sum(v[2] for v in vs) // k // 1024
            if not (0 <= xi < r.xs and 0 <= zi < r.zs):
                continue
            s = r.sectors[xi * r.zs + zi]
            below = rooms[s[2]] if s[2] != 255 else None
            if below and (below.flags & 1 or below.alt not in (-1, 0xFFFF) and rooms[below.alt].flags & 1):
                water.add(ri)
            elif abs(sum(ys) // k - s[3] * 256) <= 64:
                mist.add(ri)
    return mist, water


def main():
    path = sys.argv[1]
    b = open(path, "rb").read()
    mist, water = classify(b)
    assert not mist & water, sorted(mist & water)
    own = STRENGTH.get(os.path.basename(path).lower(), {})
    assert set(own) <= mist, sorted(set(own) - mist)
    strength = {ri: own.get(ri, 128) for ri in mist}
    if "--list" in sys.argv:
        print(strength)
        return
    _, rooms, start, end = S.parse_rooms(b)
    assert not any(r.flags & (MIST | STRENGTH_BITS) for r in rooms)
    for ri, st in strength.items():
        rooms[ri].flags = room_flags(rooms[ri].flags, st)
    open(path, "wb").write(b[:start] + S.build_rooms(rooms) + b[end:])
    print(f"{path}: mist rooms {dict(sorted((ri, st) for ri, st in strength.items() if st))}")


if __name__ == "__main__":
    main()
