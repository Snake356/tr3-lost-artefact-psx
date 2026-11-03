#include "tool.h"

#include <map>

namespace {
std::map<std::string, ToolFn> &registry() {
    static std::map<std::string, ToolFn> r;
    return r;
}
}

ToolRegistration::ToolRegistration(const char *name, ToolFn fn) { registry()[name] = std::move(fn); }

const ToolFn *find_tool(const std::string &name) {
    auto it = registry().find(name);
    return it == registry().end() ? nullptr : &it->second;
}

std::vector<std::string> tool_names() {
    std::vector<std::string> out;
    for (auto &kv : registry()) out.push_back(kv.first);
    return out;
}

std::string run_level_tool(const std::string &name, const std::vector<std::string> &args, const fs::path &cwd) {
    const ToolFn *fn = find_tool(name);
    if (!fn) throw Error("unknown level tool " + name);
    ToolContext ctx{cwd, {}};
    try {
        (*fn)(ctx, args);
    } catch (const std::exception &e) {
        throw Error(name + ": " + e.what());
    }
    return ctx.out;
}
