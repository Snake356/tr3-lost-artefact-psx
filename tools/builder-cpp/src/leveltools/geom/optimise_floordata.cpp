// optimise_floordata.py: reads every sector's floor data as trmod commands, removes all floor data, then adds it
// again longest command first (trmod shares identical sequences).
//     optimise_floordata.py <trmod executable> <level.tr2>
#include <algorithm>
#include <format>

#include "geom.h"

namespace geom {

namespace {

constexpr size_t BATCH = 8000;      // the shell buffer size of convert.sh

void optimise_floordata(ToolContext &ctx, const std::vector<std::string> &args) {
    if (args.size() < 2) throw Error("usage: optimise_floordata.py <trmod executable> <level.tr2>");
    const std::string &exe = args[0], &level = args[1];
    auto run = [&](std::vector<std::string> cmds) {
        cmds.insert(cmds.begin(), {"tr3pc", level});
        RunResult r = run_program(ctx.path(exe), cmds, ctx.cwd, {}, true);
        if (r.exit_code != 0) throw Error("trmod failed: " + r.out);
        return r.out;
    };
    std::vector<std::string> cmds = replacefloordata_commands(run({"cgetfloordata(*,*,*)"}));
    if (cmds.empty()) throw Error("no floor data commands read -- aborting before removing anything");
    std::stable_sort(cmds.begin(), cmds.end(), [](const std::string &a, const std::string &b) { return a.size() > b.size(); });
    run({"removefloordata(*,*,*)"});
    std::vector<std::string> batch;
    size_t size = 0;
    for (auto &c : cmds) {
        if (!batch.empty() && size + c.size() + 1 > BATCH) {
            run(batch);
            batch.clear();
            size = 0;
        }
        batch.push_back(c);
        size += c.size() + 1;
    }
    if (!batch.empty()) run(batch);
    ctx.print(std::format("{}: {} floor data commands re-added", level, cmds.size()));
}

}  // namespace

REGISTER_TOOL("optimise_floordata.py", optimise_floordata);

}  // namespace geom
