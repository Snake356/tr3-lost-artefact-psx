// The level tools (the steps of the level conversion and of the disc files that are not one of the
// conversion programs). Each takes the command line its convert.sh line gives, runs in-process and in
// parallel with other levels, so it keeps no global state and resolves paths against ctx.cwd.
#pragma once
#include <functional>
#include <string>
#include <vector>

#include "../core/files.h"
#include "../core/process.h"

struct ToolContext {
    fs::path cwd;          // the level's work folder
    std::string out;       // what the tool reports (as the Python tool printed it)

    fs::path path(const std::string &arg) const { return cwd / u8path(arg); }
    void print(const std::string &line) { out += line + "\n"; }
};

using ToolFn = std::function<void(ToolContext &, const std::vector<std::string> &args)>;

// name: the tool's name as convert.sh calls it, e.g. "add_crawl_flip.py".
struct ToolRegistration {
    ToolRegistration(const char *name, ToolFn fn);
};

#define REGISTER_TOOL(name, fn) static ToolRegistration tool_registration_##fn(name, fn)

const ToolFn *find_tool(const std::string &name);
std::vector<std::string> tool_names();

// Runs a tool; throws Error naming the tool when it fails.
std::string run_level_tool(const std::string &name, const std::vector<std::string> &args, const fs::path &cwd);
