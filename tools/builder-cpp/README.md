# TR3 Lost Artefact PS1 Builder (C++)

A C++20 rewrite of the Lost Artefact PS1 disc builder (the Python/Tkinter builder in
`tools/builder`, which stays for `make_data.py` and as the reference). No Python: the window, the build, the project's level tools and the disc
writer are C++; the conversion programs (tr3pcpsx, trmod, troi, trsfx, tr3mesh, texmod, 3texex, 3texrep) are
built from their C sources. It is shipped as a plain folder of files, nothing is unpacked at run time:

    TR3 Lost Artefact PS1 Builder.exe   the builder (resources: icon, manifest, version only)
    data.pak                            the project's data (also found in data\data.pak)
    tools\*.exe                         the conversion programs, started from there
    msvcp140*.dll, vcruntime140*.dll... the Visual C++ runtime (vcruntime140*.dll also in tools\)
    README.txt, CHANGES.txt             the players' readme and list of changes (tools/builder/package)
    covers\                             the three covers
    licenses\GPL-3.0.txt                the licence of the conversion programs

## Building

Visual Studio 2026 (C++ workload), CMake >= 3.25. The project is the repository this folder is in
(`tools/builder-cpp`, or pass `-DTR3LA_PROJECT_DIR=...`): its patches, scripts, layouts and pictures and the
builder's data of `tools/builder` (made by `tools/builder/make_data.py`) are packed into data.pak.

    powershell -File build.ps1                  # Release -> build\Release\ and build\dist\
    powershell -File build.ps1 -Config Debug    # Debug -> build\Debug\ (no dist)
    powershell -File dist.ps1                   # build\dist\ -> build\TR3-Lost-Artefact-PS1-Builder-v<version>.zip
    powershell -File dist.ps1 -BuildDir build2 -Build

`build\Release\` works like the distribution (data.pak and tools\ are copied next to the EXE); `build\dist\` is
the finished folder for players. The version is set once, in `project(VERSION)` in CMakeLists.txt: it goes into
the EXE's VERSIONINFO, its manifest, the window and the ZIP name (1.12.0 is shown as 1.12).

The solution `build\TR3LostArtefactPS1Builder.slnx` opens in Visual Studio after the first run.

### C runtime

The builder and the conversion programs use the C runtime as DLL (`/MD`). Nine programs share one runtime
instead of carrying nine static copies, and the runtime gets Microsoft's fixes like any other VC++ program. The
DLLs (found by CMake's `InstallRequiredSystemLibraries`) are put into dist\ app-locally, so players install
nothing; Windows looks for a program's DLLs in its own folder, so the vcruntime DLLs are in tools\ as well. Who
removes them needs the Microsoft Visual C++ v14 Redistributable (x64) instead.

## Layout

| Folder | What |
|---|---|
| `src/core` | bytes, CRC32, files, starting programs, deflate/zip, xz (LZMA2), IPS/LAD1/texture patches, JSON |
| `src/build` | reading disc images, recognising the inputs, disc files, level conversion (`levels/convert.sh` read at run time), executable/NTSC/texts, disc writer (replaces mkpsxiso), the build |
| `src/leveltools` | the project's Python tools in C++, registered under their Python names (`anim`, `geom`, `psx` groups) |
| `src/gui` | the window (Win32, painted in the dark style) |
| `src/app` | entry point, command line modes, resources (version.h and the manifest are generated from templates) |
| `cmake` | the dist folder (`dist.cmake`) |
| `packer` | `packdata`: collects data.pak |
| `external` | the conversion programs' C sources (two MSVC fixes: `trmodio.h` variadic macro, `curfaces` initialised in tr3pcpsx) |
| `scripts` | checks against the Python builder |

## Command line

    <exe> --build <TR3 PS1 image> <Lost Artefact PC files> <out .bin> [--ntsc <US image>] [--blood red|purple]
          [--level-select] [--japan-weapons] [--japan-crystals] [--keep-work] [--work <folder>]
    <exe> --tool <name.py> <args>        one level tool, as the Python tool
    <exe> --tools                        the level tools
    <exe> --write-disc <layout.xml> <out.bin> [unix time]
    <exe> --xz | --bindiff | --ips | --unzip | --anim-rebuild ...   (checks of the parts)
    <exe> --window-picture <out.png> [details]

## Checks against the Python builder

Both take the EXE of `build\Release\` or `build\dist\` (data.pak and tools\ must be next to it).

- `scripts/verify_level_tools.py <exe>`: every Python tool call of the level conversion is run in Python and
  in C++ on the same input and the results are compared byte for byte.
- `scripts/compare_builders.py <exe> <out folder>`: builds discs with both builders and compares the images
  sector by sector (the new image rewritten with the old one's build time; only file dates in directory sectors
  may differ). The game files are named by environment variables: `TR3LA_PS1_DIR` (a folder of redump zips,
  one folder per image), `TR3LA_REMASTERED` and `TR3LA_GOLD`.
