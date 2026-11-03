#pragma once

// The builder window: pick the Tomb Raider III PS1 disc and the Lost Artefact PC files, the builder recognises
// them and writes the Lost Artefact PS1 disc image. Returns the exit code of the message loop.
int run_builder_window();

// Developer mode: opens the window, saves it as a PNG picture (details shown when details is true) and closes it.
int save_window_picture(const wchar_t *png, bool details);
