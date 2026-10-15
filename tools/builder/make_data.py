"""Brings the builder's data up to date with the project after the levels, textures, animations, meshes or
texts were changed, then checks that the builder makes exactly the project's files.

  levels   levels/in/*.tr2 as patches from every known PC version (levels.json, levels/*.patch);
           only levels whose input changed since the last run (input_crc) need the PC files
  texts    tombpsx/out/<layout>.dat and art/kanji/kanji_la.psx as patches from the PS1 discs' own files
           (texts.json, texts/*.patch); only changed files need the discs
  assets   levels/textures, levels/anims, levels/mesh (assets.py make)
  check    every PC version gives levels/in, every disc gives the project's texts, and the conversion with
           the builder's hooks gives levels/out (assets.py check); levels/out must be up to date first

The command line tools, convert.sh, the geom scripts, pictures and IPS patches are read from the project
when the EXE is built (build_exe.py) and need nothing here.

The player-side files are listed in a JSON file outside the repository: {"pc": [PC disc images or game
folders], "ps1": [Tomb Raider III PS1 images]}.

Usage: make_data.py <sources.json> [--no-check]"""
import argparse
import json
import os
import sys
import zlib

import assets
import bindiff
import build
import convert
import inputs
import pc_levels
import texts

HERE = os.path.dirname(os.path.abspath(__file__))
PRJ = convert.PRJ


def crc(b):
    return "%08x" % zlib.crc32(b)


def rd(*p):
    return open(os.path.join(*p), "rb").read()


def pc_sources(paths):
    out = []
    for p in paths:
        files = inputs.pc_files_from_folder(p) if os.path.isdir(p) else inputs.pc_files_from_disc(inputs.open_image(p)[0])
        out.append((p, files))
    return out


def ps1_discs(paths):
    """{layout: disc} of the PS1 images (the first image of each layout)."""
    out = {}
    for p in paths:
        disc, _ = inputs.open_image(p)
        info = inputs.recognise_ps1(disc)
        if info and info["supported"]:
            out.setdefault(build.LAYOUT[info["version"]], disc)
    return out


def make_levels(sources):
    table = json.load(open(os.path.join(HERE, "levels.json"), encoding="utf-8"))
    pcs = None
    for lv in inputs.LEVELS:
        target = rd(PRJ, "levels", "in", f"{lv}.tr2")
        entry = table[lv]
        if entry.get("input_crc") == crc(target):
            continue
        pcs = pcs or pc_sources(sources["pc"])
        new = {"input_crc": crc(target)}
        for path, files in pcs:
            src = files.get(lv + ".tr2")
            if src is None or crc(src) in new:
                continue
            if src == target:
                new[crc(src)] = "same"
            else:
                name = f"{lv}_{crc(src)}.patch"
                p = bindiff.make(src, target)
                assert bindiff.apply(src, p) == target
                open(os.path.join(HERE, "levels", name), "wb").write(p)
                new[crc(src)] = name
        lost = [c for c in entry if c != "input_crc" and c not in new]
        if lost:
            sys.exit(f"{lv}: no PC files for the known version(s) {', '.join(lost)} in the sources list")
        for c, how in entry.items():
            if how not in ("same",) and c != "input_crc" and how not in new.values():
                os.remove(os.path.join(HERE, "levels", how))
        table[lv] = new
        print(f"levels: {lv} - {len(new) - 1} PC versions")
    u = rd(PRJ, "levels", "in", "undersea.tr2")
    u2 = rd(PRJ, "levels", "in", "underse2.tr2")
    old = table["underse2"].get("from_undersea") if isinstance(table.get("underse2"), dict) else None
    if u2 != u:
        p = bindiff.make(u, u2)
        open(os.path.join(HERE, "levels", "underse2_from_undersea.patch"), "wb").write(p)
        table["underse2"] = {"from_undersea": "underse2_from_undersea.patch"}
        print("levels: underse2 differs from undersea - patch written")
    else:
        if old:
            os.remove(os.path.join(HERE, "levels", old))
        table["underse2"] = "copy of undersea"
    json.dump(table, open(os.path.join(HERE, "levels.json"), "w", encoding="utf-8"), indent=1)


def text_target(key):
    return rd(PRJ, "art", "kanji", "kanji_la.psx") if key == "kanji" else rd(PRJ, "tombpsx", "out", f"{key}.dat")


def make_texts(sources):
    table = json.load(open(os.path.join(HERE, "texts.json"), encoding="utf-8"))
    discs = None
    for key, e in table.items():
        target = text_target(key)
        if e.get("target_crc") == crc(target):
            continue
        discs = discs or ps1_discs(sources["ps1"])
        base = e.get("base_disc", "jp")
        if base not in discs:
            sys.exit(f"texts: {key} changed, but no {base} disc in the sources list")
        src = discs[base].file(e["base_file"])
        p = bindiff.make(src, target)
        assert bindiff.apply(src, p) == target
        open(os.path.join(HERE, "texts", e["patch"]), "wb").write(p)
        e["target_crc"] = crc(target)
        print(f"texts: {key} - patch written")
    json.dump(table, open(os.path.join(HERE, "texts.json"), "w", encoding="utf-8"), indent=1)


def check(sources):
    # the tables as written by make_levels/make_texts (the modules read them when imported)
    pc_levels.TABLE = json.load(open(os.path.join(HERE, "levels.json"), encoding="utf-8"))
    texts.TABLE = json.load(open(os.path.join(HERE, "texts.json"), encoding="utf-8"))
    bad = []
    for path, files in pc_sources(sources["pc"]):
        for name, data in pc_levels.prepare(files).items():
            if data != rd(PRJ, "levels", "in", name):
                bad.append(f"{path}: {name}")
    discs = ps1_discs(sources["ps1"])
    for layout, disc in discs.items():
        if texts.tombpsx(layout, disc) != text_target(layout):
            bad.append(f"texts {layout}")
    for layout in texts.SAVE_FROM_INVENTORY:
        if layout in discs and texts.tombpsx(layout, discs[layout], test=True) != text_target(layout + "test"):
            bad.append(f"texts {layout}test")
    if "jp" in discs and texts.kanji(discs["jp"]) != text_target("kanji"):
        bad.append("kanji")
    print("levels and texts:", "all the same as the project" if not bad else f"DIFFERENT: {bad}")
    assets.check()
    return not bad


if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("sources")
    ap.add_argument("--no-check", action="store_true")
    a = ap.parse_args()
    src = json.load(open(a.sources, encoding="utf-8"))
    make_levels(src)
    make_texts(src)
    print("assets (textures, animations, meshes):")
    assets.make()
    if not a.no_check:
        check(src)
