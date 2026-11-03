// Whole-file reading and writing, paths as UTF-8 strings or std::filesystem paths.
#pragma once
#include <filesystem>
#include <string>

#include "bytes.h"

namespace fs = std::filesystem;

Bytes read_file(const fs::path &p);
void write_file(const fs::path &p, const Bytes &b);      // creates the parent folders
std::string read_text(const fs::path &p);                 // UTF-8, CR removed
fs::path u8path(const std::string &s);                    // a UTF-8 string as a path
std::string path_str(const fs::path &p);                  // a path as UTF-8
std::string lower(std::string s);
std::string upper(std::string s);
bool ends_with(const std::string &s, const std::string &tail);
bool starts_with(const std::string &s, const std::string &head);
std::vector<std::string> split(const std::string &s, char sep);
std::string trim(const std::string &s);
