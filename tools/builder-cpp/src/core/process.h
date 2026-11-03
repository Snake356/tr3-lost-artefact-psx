// Starting the conversion programs (tr3pcpsx, trmod, ...) without a console window.
#pragma once
#include <map>
#include <string>
#include <vector>

#include "files.h"

struct RunResult {
    int exit_code = 0;
    std::string out;     // stdout and stderr together
};

// Runs exe with args in cwd; env: extra environment variables; stdout_only: stderr is dropped (for programs
// whose output is data, as tr3mesh's mesh text). Waits for it to end.
RunResult run_program(const fs::path &exe, const std::vector<std::string> &args, const fs::path &cwd,
                      const std::map<std::string, std::string> &env = {}, bool stdout_only = false);

fs::path exe_folder();                                    // the folder of the builder EXE

// The conversion programs are files in the tools folder next to the builder EXE.
fs::path program_dir();                                   // <exe folder>/tools
fs::path program_path(const std::string &name);           // "trmod" -> <exe folder>/tools/trmod.exe; throws when missing

// Runs one of the conversion programs; throws Error with the end of its output when it fails.
std::string run_tool_program(const std::string &name, const std::vector<std::string> &args, const fs::path &cwd,
                             const std::map<std::string, std::string> &env = {});
