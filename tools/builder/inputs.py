"""Reads and recognises the player's own game files for the Lost Artefact PS1 builder.

PS1 side: a Tomb Raider III disc image (.cue/.bin, a single .bin, or a .zip holding them). It is recognised
by the CRC32 of its game executable (SLxS_xxx.xx, or TOMB3.EXE on the US Rev 1/2 discs whose SLUS file is
only a loader), so dumps that differ elsewhere (other tools, other sub-channel data) are recognised too.
PC side: the Lost Artefact levels (SCOTLAND, WILLSDEN, CHUNNEL, UNDERSEA, ZOO, SLINC .TR2) and MAIN.SFX,
from the PC disc (.iso/.bin/.cue/.zip) or an installed game folder (also Tomb Raider I-III Remastered,
whose Lost Artefact files are in 3/DATA/LA and 3/SFX).

All known files are listed in known_inputs.json. Nothing is written.

Usage: inputs.py <file or folder> [...]   (prints what was recognised)"""
import json
import os
import shutil
import struct
import sys
import zipfile
import zlib

HERE = os.path.dirname(os.path.abspath(__file__))
KNOWN = json.load(open(os.path.join(HERE, "known_inputs.json"), encoding="utf-8"))
LEVELS = ["scotland", "willsden", "chunnel", "undersea", "zoo", "slinc"]


class Disc:
    """ISO 9660 file system on a disc image: 2048-byte sectors (.iso) or raw 2352-byte sectors (.bin,
    mode 1 or mode 2). `f` is any seekable binary file object (also a member of a zip)."""

    def __init__(self, f):
        self.f = f
        self.ss = self.off = None
        for ss, off in ((2352, 24), (2352, 16), (2048, 0)):
            f.seek(16 * ss + off + 1)
            if f.read(5) == b"CD001":
                self.ss, self.off = ss, off
                break
        if self.ss is None:
            raise ValueError("no ISO 9660 file system found")
        if self.ss == 2352:                       # mode 1 or mode 2: the data offset follows the mode byte
            f.seek(16 * 2352 + 15)
            self.off = 16 if f.read(1) == b"\x01" else 24
        pvd = self.read(16, 2048)
        self.files = dict(self._walk(struct.unpack_from("<I", pvd, 158)[0], struct.unpack_from("<I", pvd, 166)[0]))

    def read(self, lba, size):
        n = (size + 2047) // 2048
        self.f.seek(lba * self.ss + self.off)                      # one read for all sectors
        raw = self.f.read((n - 1) * self.ss + 2048) if n else b""
        if self.ss == 2048:
            return raw[:size]
        return b"".join(raw[i * self.ss:i * self.ss + 2048] for i in range(n))[:size]

    def _walk(self, lba, size, prefix=""):
        data, p = self.read(lba, size), 0
        while p < len(data):
            ln = data[p]
            if ln == 0:
                p = (p // 2048 + 1) * 2048
                continue
            name = data[p + 33:p + 33 + data[p + 32]]
            flb, fsz, flags = struct.unpack_from("<I", data, p + 2)[0], struct.unpack_from("<I", data, p + 10)[0], data[p + 25]
            p += ln
            if name in (b"\x00", b"\x01"):
                continue
            nm = name.decode("latin-1").split(";")[0].upper()
            if flags & 2:
                yield from self._walk(flb, fsz, prefix + nm + "/")
            else:
                yield prefix + nm, (flb, fsz)

    def file(self, path):
        lba, size = self.files[path.upper()]
        return self.read(lba, size)


UNPACKED = []                                       # open unpacked images (close_unpacked)


def close_unpacked():
    while UNPACKED:
        UNPACKED.pop().close()


def open_image(path, unpack_to=None):
    """Opens a disc image given as .cue, .bin, .iso or .zip; returns (Disc, description). unpack_to: a folder
    to unpack a zipped image into first (reading inside a zip is slow wherever the reads jump back)."""
    low = path.lower()
    if low.endswith(".zip"):
        z = zipfile.ZipFile(path)
        members = [i for i in z.infolist() if i.filename.lower().endswith((".bin", ".iso", ".img"))]
        if not members:
            raise ValueError("the zip holds no .bin/.iso image")
        member = max(members, key=lambda i: i.file_size)      # the data track is the big one
        how = f"{os.path.basename(path)} -> {member.filename}"
        if unpack_to is None:
            return Disc(z.open(member)), how
        os.makedirs(unpack_to, exist_ok=True)
        dst = os.path.join(unpack_to, f"{len(os.listdir(unpack_to))}.bin")
        with z.open(member) as src, open(dst, "wb") as out:
            shutil.copyfileobj(src, out, 1 << 20)
        UNPACKED.append(open(dst, "rb"))
        return Disc(UNPACKED[-1]), how
    if low.endswith(".cue"):
        folder = os.path.dirname(path)
        for line in open(path, encoding="latin-1"):
            if line.strip().upper().startswith("FILE"):
                name = line.split('"')[1] if '"' in line else line.split()[1]
                return Disc(open(os.path.join(folder, name), "rb")), f"{os.path.basename(path)} -> {name}"
        raise ValueError("the cue sheet names no file")
    return Disc(open(path, "rb")), os.path.basename(path)


def recognise_ps1(disc):
    """The known_inputs entry of this Tomb Raider III disc, or None. Matches on the game executable."""
    for exe in ("TOMB3.EXE",) + tuple(p for p in disc.files if "/" not in p and p[:4] in ("SLUS", "SLES", "SLPM", "SCPS")):
        if exe not in disc.files:
            continue
        crc = "%08x" % zlib.crc32(disc.file(exe))
        hits = [i for i in KNOWN["ps1_images"] if i["exe"] == exe and i["exe_crc32"] == crc]
        # discs with the same executable (US Rev 1/2) are told apart by another file; Europe Rev 1 and its
        # (EDC) dump differ only in raw sector checksums and count as the same disc
        marked = [i for i in hits if "marker" in i and i["marker"]["file"] in disc.files
                  and "%08x" % zlib.crc32(disc.file(i["marker"]["file"])) == i["marker"]["crc32"]]
        if marked or hits:
            return (marked or hits)[0]
    return None


def pc_files_from_disc(disc):
    """{name: bytes} of the Lost Artefact levels and MAIN.SFX on a PC disc image."""
    out = {}
    for path in disc.files:
        base = path.split("/")[-1].lower()
        if path.startswith("DATA/") and (base[:-4] in LEVELS and base.endswith(".tr2") or base == "main.sfx"):
            out[base] = disc.file(path)
    return out


def pc_files_from_folder(folder):
    """{name: bytes} from an installed game. The folder may be the game folder or its level folder:
    <folder>/data (Gold CD install), <folder>/3/DATA/LA and 3/SFX (Tomb Raider I-III Remastered),
    TRX (games/tr3-la/levels, main.sfx one folder up), or the folder holding the files itself; the
    folder above is searched as well, for MAIN.SFX next to the level folder."""
    folder = os.path.abspath(folder)
    up = os.path.dirname(folder)
    candidates = [folder, os.path.join(folder, "data"), os.path.join(folder, "DATA"), os.path.join(folder, "levels"),
                  os.path.join(folder, "3", "DATA", "LA"), os.path.join(folder, "DATA", "LA"),
                  os.path.join(folder, "games", "tr3-la"), os.path.join(folder, "games", "tr3-la", "levels"),
                  os.path.join(folder, "3", "SFX"), os.path.join(folder, "SFX"), up, os.path.join(up, "SFX"),
                  os.path.join(os.path.dirname(up), "SFX")]           # Remastered: 3/DATA/LA -> 3/SFX
    out = {}
    for c in candidates:
        if not os.path.isdir(c):
            continue
        for f in os.listdir(c):
            low = f.lower()
            if (low.endswith(".tr2") and low[:-4] in LEVELS or low == "main.sfx") and low not in out:
                out[low] = open(os.path.join(c, f), "rb").read()
    return out


def recognise_pc(files):
    """For each level: (release name or None, crc); for MAIN.SFX the same."""
    res = {}
    for lv in LEVELS:
        data = files.get(lv + ".tr2")
        if data is None:
            res[lv] = (None, None)
            continue
        crc = "%08x" % zlib.crc32(data)
        res[lv] = (next((r for r, t in KNOWN["pc_levels"].items() if t[lv] == crc), None), crc)
    sfx = files.get("main.sfx")
    if sfx is None:
        res["main.sfx"] = (None, None)
    else:
        crc = "%08x" % zlib.crc32(sfx)
        res["main.sfx"] = (next((r for r, c in KNOWN["pc_main_sfx"].items() if c == crc), None), crc)
    return res


def describe(path):
    lines = []
    if os.path.isdir(path):
        files = pc_files_from_folder(path)
        kind = "PC folder"
    else:
        disc, how = open_image(path)
        ps1 = recognise_ps1(disc)
        if ps1:
            state = "can be built" if ps1["supported"] else "recognised, needs the revision patch (not available yet)"
            return [f"PS1: {how}", f"  Tomb Raider III {ps1['language']}, {ps1['revision']} ({ps1['redump']})", f"  {state}"]
        files = pc_files_from_disc(disc)
        kind = f"PC disc: {how}"
        if not files:
            return [f"{how}: neither a known Tomb Raider III PS1 disc nor a Lost Artefact PC disc"]
    lines.append(kind)
    for name, (release, crc) in recognise_pc(files).items():
        lines.append(f"  {name:9s} " + ("missing" if crc is None else f"{crc}  {release or 'UNKNOWN version'}"))
    return lines


if __name__ == "__main__":
    for p in sys.argv[1:]:
        try:
            print("\n".join(describe(p)))
        except (OSError, ValueError) as e:
            print(f"{p}: {e}")
