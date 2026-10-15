"""Ports the hand-made IPS patches (bin/patches/<Rev 1 version>/*.ips) to a Rev 0 executable.

Each patch record is found in the Rev 0 executable by its surroundings in the Rev 1 executable (16 words
before and after plus the original words), comparing instructions with their address parts masked (jump
targets, lui, gp-relative and other address immediates), as the code moved between the revisions. The
patched words are then carried over:
  - a word the patch leaves as it was takes the Rev 0 original word (its addresses are already right);
  - a j/jal gets its target looked up in Rev 0 (the target function found the same way);
  - a word whose immediate changed (a table offset, a constant) gets the same change on the Rev 0 word;
  - any other changed word is taken as it is (constants, branches within the record).
Records that are data (not word aligned) must appear with 32 bytes around them unchanged in Rev 0.
Usage: port_patches.py <Rev 1 exe> <Rev 0 exe> <Rev 1 patch folder> <out folder> <patch names...>"""
import os
import struct
import sys

LOAD = 0x80010000 - 0x800
CONTEXT = 16


def words(b):
    return list(struct.unpack_from(f"<{(len(b) - 0x800) // 4}I", b, 0x800))


def mask(w):
    op, rs = w >> 26, (w >> 21) & 31
    if op in (2, 3):                       # j, jal: target
        return w & 0xFC000000
    if op == 15:                           # lui: upper address half
        return w & 0xFFFF0000
    if op in (8, 9, 13) and rs not in (0, 29):          # addi/addiu/ori on a register: maybe an address low half
        return w & 0xFFFF0000
    if op in (32, 33, 34, 35, 36, 37, 38, 40, 41, 42, 43, 46, 50, 58):     # load/store
        imm = w & 0xFFFF
        if rs in (1, 28) or imm & 0x8000 or imm >= 0x1000:   # absolute ($at after lui), gp-relative, far offsets
            return w & 0xFFFF0000
    return w


def find(W1, i0, n, W0):
    """Index in W0 of the words W1[i0:i0+n] with their surroundings, or None."""
    lo, hi = max(0, i0 - CONTEXT), min(len(W1), i0 + n + CONTEXT)
    pat = [mask(w) for w in W1[lo:hi]]
    # anchor on the most distinctive unmasked word to keep the search fast
    anchors = [k for k, (w, m) in enumerate(zip(W1[lo:hi], pat)) if w == m and w not in (0,)]
    a = anchors[len(anchors) // 2] if anchors else 0
    hits = []
    for j in range(len(W0)):
        if W0[j] != pat[a]:
            continue
        s = j - a
        if s < 0 or s + len(pat) > len(W0):
            continue
        if all(mask(W0[s + k]) == p for k, p in enumerate(pat)):
            hits.append(s + (i0 - lo))
            if len(hits) > 1:
                return None
    return hits[0] if hits else None


def port_bytes(b1, b0, off, data):
    """Data (not code): the record with 32 bytes before and after must appear once, unchanged, in Rev 0."""
    for before in (32, 0):                # pointers in front may have moved: then the record and what follows
        ctx = b1[off - before:off] + b1[off:off + len(data)] + b1[off + len(data):off + len(data) + 32]
        k = b0.find(ctx)
        if k >= 0 and b0.find(ctx, k + 1) < 0:
            return (k + before, data), None
    return None, "data surroundings not found (or not unique)"


def port_record(W1, W0, off, data, b1=None, b0=None):
    if off % 4 or len(data) % 4:
        return port_bytes(b1, b0, off, data)
    i0, n = (off - 0x800) // 4, len(data) // 4
    j0 = find(W1, i0, n, W0)
    if j0 is None:
        return port_bytes(b1, b0, off, data) if b1 else (None, "surroundings not found (or not unique)")
    new = list(struct.unpack(f"<{n}I", data))
    out = []
    for k, w in enumerate(new):
        orig1, orig0 = W1[i0 + k], W0[j0 + k]
        if w == orig1:
            out.append(orig0)
        elif w >> 26 in (2, 3):
            t1 = ((w & 0x3FFFFFF) << 2)
            ti = (t1 | 0x80000000) - 0x80010000
            tj = find(W1, ti // 4, 8, W0)
            if tj is None:
                return None, f"jump target {t1:#x} not found"
            out.append((w & 0xFC000000) | (((tj * 4 + 0x80010000) >> 2) & 0x3FFFFFF))
        elif mask(w) != w and mask(w) == mask(orig1) and mask(orig0) == mask(orig1):
            # only the immediate changed: the same change on the Rev 0 word (table offsets, constants)
            d = ((w & 0xFFFF) - (orig1 & 0xFFFF)) & 0xFFFF
            out.append((orig0 & 0xFFFF0000) | (((orig0 & 0xFFFF) + d) & 0xFFFF))
        else:
            out.append(w)
    return (0x800 + j0 * 4, struct.pack(f"<{n}I", *out)), None


def records(ips):
    p, out = 5, []
    while ips[p:p + 3] != b"EOF":
        o, n = int.from_bytes(ips[p:p + 3], "big"), int.from_bytes(ips[p + 3:p + 5], "big")
        p += 5
        if n:
            out.append((o, ips[p:p + n]))
            p += n
        else:
            r = int.from_bytes(ips[p:p + 2], "big")
            out.append((o, ips[p + 2:p + 3] * r))
            p += 3
    return out


def ips(recs):
    return b"PATCH" + b"".join(o.to_bytes(3, "big") + len(d).to_bytes(2, "big") + d for o, d in recs) + b"EOF"


if __name__ == "__main__":
    rev1, rev0, src, dst = sys.argv[1:5]
    b1, b0 = open(rev1, "rb").read(), open(rev0, "rb").read()
    W1, W0 = words(b1), words(b0)
    os.makedirs(dst, exist_ok=True)
    for name in sys.argv[5:]:
        recs, problems = [], []
        for off, data in records(open(os.path.join(src, f"{name}.ips"), "rb").read()):
            r, err = port_record(W1, W0, off, data, b1, b0)
            if r:
                recs.append(r)
            else:
                problems.append(f"{off:#x}: {err}")
        if problems:
            print(f"{name}: NOT ported - {'; '.join(problems)}")
        else:
            open(os.path.join(dst, f"{name}.ips"), "wb").write(ips(recs))
            print(f"{name}: ported ({', '.join(f'{o:#x}' for o, _ in recs)})")
