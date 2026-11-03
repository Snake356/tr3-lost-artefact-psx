// Writing a PS1 disc image from a disc layout in mkpsxiso's XML format: a .bin of raw 2352-byte mode 2 sectors
// and its .cue, byte for byte as mkpsxiso 1.26 writes them (the same LBA order, padding, sub-headers and EDC/ECC).
//
// The layout features supported are the ones the builder's layouts use: one data track, <identifiers>, <license>,
// <directory_tree> with <dir>, <file type="data|xa|str|mixed"> and <dummy sectors="n">, and the iso_project
// attributes image_name, cue_sheet and no_xa. Source paths are taken as they are (the builder writes absolute ones).
#pragma once
#include <ctime>
#include <string>

#include "../core/files.h"

// Writes out_bin and, when the layout has a cue_sheet attribute, the cue sheet of that name in out_bin's folder.
// The cue sheet names the image by the layout's image_name (out_bin's file name if there is none).
// build_time stamps the volume descriptor and the directory records, as mkpsxiso stamps the time it ran
// (directory records in UTC, the volume descriptor in local time). Throws Error on a bad layout or missing file.
void write_disc_image(const std::string &xml_text, const fs::path &out_bin, std::time_t build_time = std::time(nullptr));
