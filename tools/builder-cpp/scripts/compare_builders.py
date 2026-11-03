"""Builds the same discs with the old Python builder and the new C++ builder and compares the images.

The images are compared sector by sector; the new image is first rewritten with the old image's build time
(builder --write-disc), so only the file dates in the directory sectors may differ (they come from the times the
intermediate files were written). Each case reports the differing sectors and whether they are all directory sectors.

Usage: compare_builders.py <builder exe> <out folder> [case ...]   (cases: see CASES)
The game files come from environment variables: TR3LA_PS1_DIR (a folder with the redump zips, one folder per
image as in the No-Intro/redump sets), TR3LA_REMASTERED (the Tomb Raider I-III Remastered folder) and TR3LA_GOLD
(the Tomb Raider III Gold CD image)."""
import calendar
import os
import shutil
import subprocess
import sys

PRJ = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", ".."))
PS1 = os.environ.get("TR3LA_PS1_DIR", "") + "/Tomb Raider III - {0}/Tomb Raider III - {0}.zip"
REMASTERED = os.environ.get("TR3LA_REMASTERED", "")
GOLD = os.environ.get("TR3LA_GOLD", "")
US1 = PS1.format("Adventures of Lara Croft (USA) (Rev 1)")
CASES = {
    "de_ntsc": [PS1.format("Adventures of Lara Croft (Germany) (Rev 1)"), REMASTERED, "--ntsc", US1, "--blood", "red"],
    "jp1_select": [PS1.format("Adventures of Lara Croft (Japan) (Disc 1) (Japanese Version)"), REMASTERED, "--level-select"],
    "uk0_japan": [PS1.format("Adventures of Lara Croft (Europe)"), REMASTERED, "--japan-weapons", "--japan-crystals"],
    "as2_gold": [PS1.format("Adventures of Lara Croft (Asia) (Disc 2) (International Version)"), GOLD, "--blood", "purple"],
    "us0_select": [PS1.format("Adventures of Lara Croft (USA)"), GOLD, "--level-select"],
}
S = 2352


def build_time(path):
    with open(path, "rb") as f:
        f.seek(16 * S + 24)
        pvd = f.read(2048)
        f.seek(int.from_bytes(pvd[158:162], "little") * S + 24)
        y, mo, d, h, mi, s = f.read(64)[18:24]
    return calendar.timegm((1900 + y, mo, d, h, mi, s, 0, 0, 0))


def directory_sectors(path):
    """LBAs of all directory extents (walk from the root)."""
    out = set()
    with open(path, "rb") as f:
        def rd(lba, n):
            data = b""
            for i in range((n + 2047) // 2048):
                f.seek((lba + i) * S + 24)
                data += f.read(2048)
            return data[:n]
        pvd = rd(16, 2048)
        todo = [(int.from_bytes(pvd[158:162], "little"), int.from_bytes(pvd[166:170], "little"))]
        while todo:
            lba, size = todo.pop()
            if lba in out:
                continue
            out.update(range(lba, lba + (size + 2047) // 2048))
            d, p = rd(lba, size), 0
            while p < len(d):
                ln = d[p]
                if ln == 0:
                    p = (p // 2048 + 1) * 2048
                    continue
                name = d[p + 33:p + 33 + d[p + 32]]
                if d[p + 25] & 2 and name not in (b"\0", b"\1"):
                    todo.append((int.from_bytes(d[p + 2:p + 6], "little"), int.from_bytes(d[p + 10:p + 14], "little")))
                p += ln
    return out


def compare(a, b):
    sa, sb = os.path.getsize(a), os.path.getsize(b)
    if sa != sb:
        return f"DIFFERENT size {sa} / {sb}"
    dirs = directory_sectors(b)
    bad = []
    with open(a, "rb") as fa, open(b, "rb") as fb:
        lba = 0
        while True:
            x, y = fa.read(S * 4096), fb.read(S * 4096)
            if not x:
                break
            if x != y:
                for i in range(len(x) // S):
                    if x[i * S:(i + 1) * S] != y[i * S:(i + 1) * S]:
                        bad.append(lba + i)
            lba += len(x) // S
    other = [s for s in bad if s not in dirs]
    if other:
        return f"DIFFERENT: {len(other)} data sectors differ (first {other[:10]}), {len(bad) - len(other)} directory sectors"
    return f"SAME data ({len(bad)} directory sectors differ: file dates)"


def main():
    exe, out = os.path.abspath(sys.argv[1]), os.path.abspath(sys.argv[2])
    names = sys.argv[3:] or list(CASES)
    results = []
    for name in names:
        args = CASES[name]
        folder = os.path.join(out, name)
        shutil.rmtree(folder, ignore_errors=True)
        os.makedirs(folder)
        old, new = os.path.join(folder, "old", f"{name}.bin"), os.path.join(folder, "new", f"{name}.bin")
        r1 = subprocess.run(["py", "-3", PRJ + "/tools/builder/build.py", args[0], args[1], old] + args[2:],
                            capture_output=True, text=True, errors="replace")
        r2 = subprocess.run([exe, "--build", args[0], args[1], new] + args[2:] + ["--keep-work"],
                            capture_output=True, text=True, errors="replace")
        if r1.returncode or r2.returncode:
            res = f"BUILD FAILED old={r1.returncode} new={r2.returncode}\n{r1.stdout[-600:]}{r1.stderr[-600:]}\n{r2.stdout[-600:]}"
        else:
            same = os.path.join(folder, "same_time")
            os.makedirs(same)
            xml = open(os.path.join(folder, "new", "build_work", "disc.xml"), encoding="utf-8").read()
            open(os.path.join(same, "disc.xml"), "w", encoding="utf-8").write(xml)
            subprocess.run([exe, "--write-disc", os.path.join(same, "disc.xml"), os.path.join(same, f"{name}.bin"),
                            str(build_time(old))], check=True)
            res = compare(os.path.join(same, f"{name}.bin"), old)
            for ext in (".cue", ".png"):
                a, b = old[:-4] + ext, os.path.join(same if ext == ".cue" else os.path.dirname(new), f"{name}{ext}")
                if open(a, "rb").read() != open(b, "rb").read():
                    res += f"; {ext} DIFFERENT"
            shutil.rmtree(os.path.join(folder, "new", "build_work"), ignore_errors=True)
        print(f"{name:12s} {res}", flush=True)
        with open(os.path.join(out, "results.txt"), "a", encoding="utf-8") as f:
            f.write(f"{name:12s} {res}\n")
        results.append(res)
    sys.exit(0 if all(r.startswith("SAME") for r in results) else 1)


if __name__ == "__main__":
    main()
