// Tomb Raider III: The Lost Artefact - PS1 disc builder.
//
// Without arguments: the builder window. Command line modes (output to the calling console):
//   --tool <name> <args...>    runs one level tool in the current folder (e.g. add_crawl_flip.py level.tr2)
//   --tools                    lists the level tools
//   --build <TR3 PS1 image> <Lost Artefact PC files> <out .bin> [--ntsc <US image>] [--blood red|purple]
//           [--level-select] [--japan-weapons] [--japan-crystals] [--keep-work] [--work <folder>]
//                              builds the disc image without the window
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>

#include <string>
#include <vector>

#include "../build/build.h"
#include "../build/data.h"
#include "../core/process.h"
#include "../gui/window.h"
#include "../leveltools/tool.h"
#include "console.h"
#include "devtools.h"

namespace {

std::string narrow(const wchar_t *w) {
    int n = WideCharToMultiByte(CP_UTF8, 0, w, -1, nullptr, 0, nullptr, nullptr);
    std::string s(size_t(n > 0 ? n - 1 : 0), '\0');
    WideCharToMultiByte(CP_UTF8, 0, w, -1, s.data(), n, nullptr, nullptr);
    return s;
}

std::vector<std::string> arguments() {
    int argc = 0;
    wchar_t **argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    std::vector<std::string> out;
    for (int i = 1; i < argc; i++) out.push_back(narrow(argv[i]));
    LocalFree(argv);
    return out;
}

int tool_mode(const std::vector<std::string> &args) {
    if (args.empty()) {
        console_write("usage: --tool <name> <args...>\n");
        return 2;
    }
    try {
        std::string out = run_level_tool(args[0], std::vector<std::string>(args.begin() + 1, args.end()), fs::current_path());
        console_write(out);
        return 0;
    } catch (const std::exception &e) {
        console_write(std::string(e.what()) + "\n");
        return 1;
    }
}

int build_mode(const std::vector<std::string> &args) {
    BuildOptions o;
    std::vector<std::string> pos;
    try {
        for (size_t i = 0; i < args.size(); i++) {
            const std::string &a = args[i];
            auto value = [&]() -> std::string {
                if (i + 1 >= args.size()) throw Error(a + " needs a value");
                return args[++i];
            };
            if (a == "--ntsc") o.ntsc_image = u8path(value());
            else if (a == "--blood") {
                o.blood = value();
                if (o.blood != "red" && o.blood != "purple") throw Error("--blood: red or purple");
            } else if (a == "--level-select" || a == "--test") o.level_select = true;
            else if (a == "--japan-weapons") o.japan_weapons = true;
            else if (a == "--japan-crystals") o.japan_crystals = true;
            else if (a == "--keep-work") o.keep_work = true;
            else if (a == "--work") o.work = u8path(value());
            else if (starts_with(a, "--")) throw Error("unknown option " + a);
            else pos.push_back(a);
        }
        if (pos.size() != 3) throw Error("usage: --build <TR3 PS1 image> <Lost Artefact PC files> <out .bin> [options]");
        o.ps1_image = u8path(pos[0]);
        o.pc_source = u8path(pos[1]);
        o.out_bin = u8path(pos[2]);
        build_disc(o, [](const std::string &s) { console_write(s + "\n"); });
        return 0;
    } catch (const std::exception &e) {
        console_write(std::string("error: ") + e.what() + "\n");
        return 1;
    }
}

}

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int) {
    std::vector<std::string> args = arguments();
    if (args.size() >= 2 && args[0] == "--window-picture")
        return save_window_picture(fs::path(u8path(args[1])).c_str(), args.size() > 2 && args[2] == "details");
    if (!args.empty() && args[0] == "--build") {
        console_attach();
        return build_mode(std::vector<std::string>(args.begin() + 1, args.end()));
    }
    if (!args.empty() && args[0] == "--tool") {
        console_attach();
        return tool_mode(std::vector<std::string>(args.begin() + 1, args.end()));
    }
    if (!args.empty() && args[0] == "--tools") {
        console_attach();
        for (auto &n : tool_names()) console_write(n + "\n");
        return 0;
    }
    if (!args.empty() && args[0].rfind("--", 0) == 0) {
        console_attach();
        int r = dev_mode(args[0], std::vector<std::string>(args.begin() + 1, args.end()));
        if (r != -1) return r;
    }
    // the window needs data.pak from the start: say so plainly instead of failing somewhere inside
    std::string problem = data_problem();
    if (!problem.empty()) {
        int n = MultiByteToWideChar(CP_UTF8, 0, problem.data(), int(problem.size()), nullptr, 0);
        std::wstring w(size_t(n), L' ');
        MultiByteToWideChar(CP_UTF8, 0, problem.data(), int(problem.size()), w.data(), n);
        MessageBoxW(nullptr, w.c_str(), L"TR3 Lost Artefact PS1 Builder", MB_OK | MB_ICONERROR);
        return 1;
    }
    return run_builder_window();
}
