// The builder's own data, the file data.pak next to the EXE (packer/packdata.cpp): own/... (patches, assets,
// tables, covers), prj/... (project files: executable patches, conversion scripts, disc layouts, pictures).
#pragma once
#include <string>
#include <vector>

#include "../core/files.h"
#include "../core/json.h"

bool data_has(const std::string &name);
const Bytes &data_file(const std::string &name);              // throws when missing
std::string data_text(const std::string &name);               // UTF-8, CR removed
const Json &data_json(const std::string &name);               // parsed once
std::vector<std::string> data_names(const std::string &prefix);

// Where data.pak is: <exe folder>\data.pak or <exe folder>\data\data.pak; empty when it is in neither.
fs::path data_pack_path();
// Why the data cannot be read (data.pak missing or damaged), empty when it can; reads the pack.
std::string data_problem();
