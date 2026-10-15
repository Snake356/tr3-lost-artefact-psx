"""Copies the public part of the project into a checkout of the public repository: the tools and their
sources, the builder, the IPS patches, disc layouts, conversion scripts, the pictures the builder ships
and the documentation. No game data: levels, sounds, speech and video, executables, game texts and fonts,
the code modules taken from the game and the unchanged pictures stay in the working repository.

Files in the target that are no longer exported are removed (except .git). The readme gets a note at the
top that the game data is not included.

Usage: export_public.py <public repository checkout>"""
import fnmatch
import os
import shutil
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
PRJ = os.path.normpath(os.path.join(HERE, ".."))
sys.path.insert(0, os.path.join(HERE, "builder"))

INCLUDE = ["readme.md", "LICENSE", ".github/*", "split_info.txt", ".gitattributes", "tools/*", "bin/patch.sh",
           "bin/patches/*", "bin/modules.txt", "disc/*.xml", "disc/files/system/*", "levels/convert.sh",
           "levels/geom/*", "levels/japan/*", "levels/sfx/empty.wav", "guides/*", "docs/*",
           "tombpsx/convert.sh"]
EXCLUDE = ["tools/bin/*", "tools/src/gameflow/*", "*/modblob.h", "tools/src/mkpsxiso/bin_nix/*",
           "tools/src/mkpsxiso/CMakeFiles/*", "*/CMakeCache.txt", "*.o", "*.res", "guides/trrs.html"]
GITIGNORE = "Thumbs.db\n.DS_Store\n*.tmp\n__pycache__/\nbackups/\nergebnisse/\nbuild/\n"
NOTE = """> **No game data is included in this repository.** The levels, sounds, speech, videos, executables
> and texts of Tomb Raider III and The Lost Artefact belong to their rights holders. To play, build the
> disc image from your own games with the disc builder (see "Playing it" below).
> The conversion pipeline described here needs those files in their places (`levels/in`, `disc/files`,
> `bin/orig` and so on), taken from your own copies.

"""


def selected():
    files = subprocess.run(["git", "ls-files"], cwd=PRJ, capture_output=True, text=True, check=True).stdout.split("\n")
    match = lambda f, pats: any(fnmatch.fnmatch(f, p) for p in pats)
    out = [f for f in files if f and match(f, INCLUDE) and not match(f, EXCLUDE)]
    import build_exe                                    # the edited pictures the builder ships
    out += [f for f in build_exe.project_files() if f.startswith("art/")]
    return sorted(set(out))


def main(target):
    files = selected()
    for root, dirs, names in os.walk(target):          # remove what is no longer exported
        dirs[:] = [d for d in dirs if d != ".git"]
        for n in names:
            rel = os.path.relpath(os.path.join(root, n), target).replace("\\", "/")
            if rel not in files and rel != ".gitignore":
                os.remove(os.path.join(root, n))
    for f in files:
        dst = os.path.join(target, f)
        os.makedirs(os.path.dirname(dst) or target, exist_ok=True)
        shutil.copyfile(os.path.join(PRJ, f), dst)
    readme = open(os.path.join(PRJ, "readme.md"), encoding="utf-8").read()
    head, rest = readme.split("\n", 1)
    open(os.path.join(target, "readme.md"), "w", encoding="utf-8").write(head + "\n\n" + NOTE + rest.lstrip("\n"))
    open(os.path.join(target, ".gitignore"), "w", encoding="utf-8").write(GITIGNORE)
    size = sum(os.path.getsize(os.path.join(target, f)) for f in files)
    print(f"{len(files)} files, {size / 1e6:.1f} MB -> {target}")


if __name__ == "__main__":
    main(sys.argv[1])
