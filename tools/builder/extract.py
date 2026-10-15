"""Extracts the files the Lost Artefact disc takes over from the player's Tomb Raider III PS1 image.

Written to <out>/:
  exe/<version>.exe        the game executable (TOMB3.EXE on US Rev 1/2, SLxS_xxx.xx elsewhere)
  DATA/*                   FMV.MOD, TITLE.PSX, HOUSE.PSX and KANJI.PSX (when present), as on the disc
  PIX/*.RAW                the pictures (loading screens, title, legal, logos)
  XA/F*.XA, FMV/LOGO.FMV   streamed audio/video as raw mode 2 sectors (2336 bytes each: subheader + data),
                           the form mkpsxiso takes for type="xa"/"str"
  SYSTEM.CNF
Usage: extract.py <image> <out folder>"""
import os
import sys
import zlib

import inputs


def raw_file(disc, path):
    """A file of mode 2 form 2 sectors (XA audio, STR video) as 2336-byte raw sectors."""
    if disc.ss != 2352:
        raise ValueError(f"{path}: streamed files need a raw (2352-byte sector) image, not an .iso")
    lba, size = disc.files[path.upper()]
    n = (size + 2047) // 2048
    disc.f.seek(lba * 2352 + 16)                                    # one read for all sectors
    raw = disc.f.read((n - 1) * 2352 + 2336) if n else b""
    return b"".join(raw[i * 2352:i * 2352 + 2336] for i in range(n))


def extract(disc, info, out):
    written = {}

    def put(rel, data):
        p = os.path.join(out, rel)
        os.makedirs(os.path.dirname(p), exist_ok=True)
        open(p, "wb").write(data)
        written[rel] = "%08x" % zlib.crc32(data)

    put(f"exe/{info['version']}.exe", disc.file(info["exe"]))
    for path in sorted(disc.files):
        top, _, name = path.rpartition("/")
        if top == "DATA" and name in ("FMV.MOD", "TITLE.PSX", "HOUSE.PSX", "KANJI.PSX"):
            put(path, disc.file(path))
        elif top.startswith("PIX") and name.endswith(".RAW"):
            put(f"PIX/{name}", disc.file(path))
        elif top == "XA" and name.endswith(".XA") or path == "FMV/LOGO.FMV":
            put(path, raw_file(disc, path))
        elif path == "SYSTEM.CNF":
            put(path, disc.file(path))
    return written


if __name__ == "__main__":
    image, out = sys.argv[1], sys.argv[2]
    disc, how = inputs.open_image(image)
    info = inputs.recognise_ps1(disc)
    if info is None:
        sys.exit(f"{how}: not a known Tomb Raider III PS1 disc")
    w = extract(disc, info, out)
    print(f"{how}: {info['language']} {info['revision']}, {len(w)} files -> {out}")
