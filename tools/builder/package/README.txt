================================================================================
  TOMB RAIDER III: THE LOST ARTEFACT - PS1
  Disc builder
================================================================================

The Lost Artefact was only ever released for the PC. This project brings it
to the PlayStation, built on the original Tomb Raider III PS1 engine: all six
levels (Sleeping with the Fishes in two parts), loading screens, credits,
savegame crystals, and all language versions of the PS1 game.

No game data is included in this download. The builder makes the PS1 disc
image from your own copies of both games:

  1. your Tomb Raider III PS1 disc image, and
  2. your Tomb Raider III: The Lost Artefact PC files.

CHANGES.txt lists what is different from the PC version and why, the known
issues and the fixed bugs.


--------------------------------------------------------------------------------
  WHAT YOU NEED
--------------------------------------------------------------------------------

1. A Tomb Raider III PS1 disc image (.bin/.cue, .iso or a .zip with them)

   Every release is supported. The disc you choose decides the language of
   your Lost Artefact disc (speech, texts, loading screens):

     USA                       original, Rev 1, Rev 2
     Europe (English)          original, Rev 1, Rev 1 (EDC)
     Germany                   original, Rev 1
     France                    original, Rev 1
     Italy                     original, Rev 1
     Spain                     original, Rev 1
     Japan                     Disc 1 (Japanese), Disc 2 (International)
     Asia                      Disc 1 (Japanese), Disc 2 (International)

   The image must be an unmodified dump (as listed by redump.org). The
   builder checks it and tells you which version it found.

2. The Lost Artefact PC files: the six levels and MAIN.SFX (the sounds)

   Any of these works:
     - the Tomb Raider III Gold / The Lost Artefact CD (any language),
       as a disc image or installed (the game folder)
     - Tomb Raider I-III Remastered: choose the game folder

   For a folder, the builder searches it for the levels and MAIN.SFX.

3. Windows 10 or 11, about 1.5 GB of free space next to where you save the
   disc image (the finished image is about 400 MB).


--------------------------------------------------------------------------------
  HOW TO USE
--------------------------------------------------------------------------------

1. Start "TR3 Lost Artefact PS1 Builder.exe".
2. Step 1: choose your Tomb Raider III PS1 disc image.
3. Step 2: choose your Lost Artefact files (a disc image or a folder).
   Both steps show a green tick when the files were recognised.
4. Step 3: options (see below), or leave them as they are.
5. Choose where to save the disc image and click "Build disc image".

Building takes about one minute. You get:

   tr3la_<version>.bin / .cue   the disc image
   tr3la_<version>.png          the matching cover (PAL, USA or Japan)

Windows may warn about an unknown program the first time ("Windows protected
your PC"): click "More info", then "Run anyway".


--------------------------------------------------------------------------------
  OPTIONS
--------------------------------------------------------------------------------

Blood
   "As the game has it" keeps the colour of your version (the German
   versions show purple blood). "Red" or "Purple" sets it for any version.

Level select on New Game (optional)
   New Game lets you start any level. For a normal playthrough, leave it
   off: a level started this way begins without what you would have
   collected before. The Japanese and Asian Disc 1 get a save crystal in
   the inventory at every level start (they save from the inventory); all
   other versions also get the cheat codes.

NTSC 60 Hz (European discs only)
   Turns a German, French, Italian, Spanish or English (Europe) disc into
   an NTSC disc with your language's speech and texts. This needs the US
   Tomb Raider III disc as well (any revision: Rev 0, 1 or 2), for the NTSC
   game code. The disc then runs like the US version (also on NTSC
   consoles).


--------------------------------------------------------------------------------
  PLAYING
--------------------------------------------------------------------------------

Emulators: open the .cue file (DuckStation, ePSXe, Beetle PSX and others).
Many front ends show the .png next to it as the cover; for DuckStation, copy
it into its "covers" folder. The cover is optional: use your own if you
prefer, or none at all.

DuckStation with PGXP: keep "Geometry Correction" on (it stops the shaking),
but switch "Texture Correction" off. With it on, the items in the inventory
ring (for example the controller) and the very top of the sky dome look
grainy and torn. Without it, textures on large surfaces close to the camera
bend a little, as on a real PlayStation.

Real PlayStation: use an optical drive emulator, or burn the .bin/.cue to a
CD-R (the console must be able to play burned discs). Use the version that
matches your console (PAL or NTSC) unless it plays both.

The Japanese and Asian Disc 1 get easier levels (more pickups and save
crystals, fewer traps), as the original Japanese Tomb Raider III did. The
Lost Artefact came out in Japan only on the PC, with the same levels as the
western version, so these levels were adapted by hand for this project (by
XeNoN3560), following the original Japanese Tomb Raider III.


--------------------------------------------------------------------------------
  IF SOMETHING GOES WRONG
--------------------------------------------------------------------------------

"Not a known Tomb Raider III PS1 disc"
   The image is not one of the releases above, or it was changed (patched,
   converted, compressed with another tool). Use a clean dump.

"missing: main.sfx"
   MAIN.SFX holds the sounds. On the Gold CD it is next to the level folder;
   in Remastered it is in 3/SFX. Choose the game folder, not the level folder.

"unknown version"
   The level files are not one of the known Lost Artefact releases (for
   example edited levels). Use the original files.

The build stops with an error
   The message is shown under "Details". The intermediate files stay in the
   folder "build_work" next to the disc image; you can delete it.
   "Report this error on GitHub" opens a report with the error message and
   your disc version (without your folder names).

A problem in the game
   Use "Report a problem" in the builder, or open an issue at
   https://github.com/Snake356/tr3-lost-artefact-psx/issues


--------------------------------------------------------------------------------
  CREDITS
--------------------------------------------------------------------------------

PS1 port                   b122251 - the end credits close with the
                           farewell message he left for the Tomb Raider
                           community
Carried the port on        FindingTheLost - listed many of the bugs fixed
                           since and came up with splitting Sleeping with
                           the Fishes in two to stay within the PS1's limits
Continued and finished by  XeNoN3560
Japanese/Asian Disc 1      XeNoN3560 - the easier levels
PS1 disc builder           XeNoN3560
Covers and icon            XeNoN3560

Claude (Anthropic) was used as an AI assistant to help finish the project.

The project continues b122251's original PS1 conversion, which b122251
published for others to finish:

   https://gitlab.com/b122251/tr3gpsx

The disc image is written with mkpsxiso by Lameguy64 (GNU GPL version 2,
see licenses/mkpsxiso-GPL-2.0.txt). The source code of the project and of
all tools is at:

   https://github.com/Snake356/tr3-lost-artefact-psx


--------------------------------------------------------------------------------
  LEGAL
--------------------------------------------------------------------------------

Tomb Raider III and The Lost Artefact (c) Core Design / Eidos Interactive;
the rights are now held by Crystal Dynamics. This is a non-commercial fan
project and is not affiliated with or endorsed by them.

No game data is included. The builder only contains the project's own
changes (as differences to your files), its own pictures and tools. You need
your own copies of both games, and the disc images you build are for your
own use only: do not share them.

If you hold rights to any of this and want something removed, please get in
touch through the project page above.
