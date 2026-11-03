#include "convert.h"

#include <atomic>
#include <cctype>
#include <map>
#include <mutex>
#include <thread>

#include "../core/patches.h"
#include "../core/process.h"
#include "../leveltools/tool.h"
#include "data.h"

namespace {

std::string setting(const std::string &script, const std::string &name) {
    for (auto &line : split(script, '\n'))
        if (starts_with(line, name + "=\"")) return line.substr(name.size() + 2, line.find('"', name.size() + 2) - name.size() - 2);
    throw Error("levels/convert.sh: no setting " + name);
}

// A command line split as a POSIX shell does (quotes and backslashes; nothing is expanded).
std::vector<std::string> shell_words(const std::string &line) {
    std::vector<std::string> out;
    std::string cur;
    bool word = false;
    for (size_t i = 0; i < line.size(); i++) {
        char c = line[i];
        if (c == ' ' || c == '\t') {
            if (word) out.push_back(cur), cur.clear(), word = false;
        } else if (c == '\'') {
            word = true;
            size_t e = line.find('\'', i + 1);
            if (e == std::string::npos) throw Error("unclosed quote in: " + line);
            cur += line.substr(i + 1, e - i - 1);
            i = e;
        } else if (c == '"') {
            word = true;
            for (i++; i < line.size() && line[i] != '"'; i++) {
                if (line[i] == '\\' && i + 1 < line.size() && std::string("\"\\$`\n").find(line[i + 1]) != std::string::npos) i++;
                cur += line[i];
            }
            if (i >= line.size()) throw Error("unclosed quote in: " + line);
        } else if (c == '\\' && i + 1 < line.size()) {
            word = true;
            cur += line[++i];
        } else {
            word = true;
            cur += c;
        }
    }
    if (word) out.push_back(cur);
    return out;
}

// The steps of the level customisation block: a command line or a loop over the levels.
struct Step {
    bool loop;
    std::string text;
};

std::vector<Step> customisation(const std::string &s) {
    size_t a = s.find("### LEVEL CUSTOMISATION ###"), b = s.find("### JAPAN / ASIA DISC 1 ###");
    if (a == std::string::npos || b == std::string::npos) throw Error("levels/convert.sh: customisation block not found");
    std::vector<Step> out;
    for (auto &line : split(s.substr(a, b - a), '\n')) {
        std::string t = trim(line);
        if ((starts_with(t, "./texmod ") || starts_with(t, "./trmod ") || starts_with(t, "py -3 \"${prjdir}/../tools/")) &&
            t.find("${level}") == std::string::npos)
            out.push_back({false, t});
        else if (starts_with(t, "./3texrep")) out.push_back({true, "textures"});
        else if (starts_with(t, "./troi")) out.push_back({true, "anims"});
        else if (starts_with(t, "sh \"./geom/")) out.push_back({true, "geom"});
        else if (starts_with(t, "./tr3mesh")) out.push_back({true, "mesh"});
        else if (starts_with(t, "fd=\"$(./trmod")) out.push_back({true, "floordata"});
    }
    return out;
}

// The level a customisation line is for (its first <name>.tr2).
std::string line_level(const std::string &t) {
    size_t e = t.find(".tr2");
    if (e == std::string::npos) return {};
    size_t a = e;
    while (a > 0 && (isalnum(uint8_t(t[a - 1])) || t[a - 1] == '_')) a--;
    return t.substr(a, e - a);
}

// Silenced samples that end: the project's empty.wav becomes a 32-byte VAG flagged 0x07 (an endless silent loop
// that keeps its sound slot busy); flagged 0x01 (end + mute) the voice stops.
void stop_empty_samples(const fs::path &psx) {
    Bytes d = read_file(psx);
    uint32_t n = u32(d, 4);
    std::vector<uint32_t> offs;
    for (uint32_t i = 0; i < n; i++) offs.push_back(u32(d, 8 + 4 * i));
    offs.push_back(u32(d, 8 + 4 * n));
    size_t base = 8 + 4 * size_t(n) + 4;
    Bytes empty(32, 0);
    empty[17] = 0x07;
    for (size_t i = 0; i + 1 < offs.size(); i++) {
        size_t a = base + offs[i], b = base + offs[i + 1];
        if (b > a && b - a == 32 && b <= d.size() && std::equal(empty.begin(), empty.end(), d.begin() + long(a)))
            d[a + 17] = 0x01;
    }
    write_file(psx, d);
}

const char *const ANIM_TOOLS[] = {"add_crawl_flip.py", "add_crouch_turn.py", "add_crawl_pickup.py",
                                  "fix_crouch_roll_arm.py", "add_controlled_drop.py", "add_ledge_jumps.py",
                                  "add_corner_shimmy.py", "add_climb_corners.py", "strip_dead_anims.py",
                                  "compact_frames.py", "pack_rotations.py"};

class LevelConverter {
public:
    LevelConverter(const ConvertJob &job, const std::string &script, const std::string &level)
        : job_(job), s_(script), lv_(level), dir_(job.work / u8path(level)) {}

    void run() {
        fs::create_directories(dir_);
        fs::copy_file(job_.inputs / u8path(lv_ + ".tr2"), dir_ / u8path(lv_ + ".tr2"));
        sounds();
        for (auto &st : customisation(s_)) {
            if (!st.loop) {
                if (line_level(st.text) == lv_) shell_line(st.text);
            } else if (st.text == "textures") {
                textures();
            } else if (st.text == "anims") {
                anims();
            } else if (st.text == "geom") {
                geom();
            } else if (st.text == "mesh") {
                mesh();
            } else if (st.text == "floordata") {
                level_tool("optimise_floordata.py", {path_str(program_path("trmod")), tr2()});
            }
        }
        japan();
        const std::string &ver = job_.version;
        bool jp = ver == "jp" || ver == "as";
        std::string src = jp ? lv_ + "_jp.tr2" : tr2();
        std::vector<std::string> args{src, "./out/" + ver + "/" + lv_ + ".psx", "./outsfx/" + ver + "/" + lv_ + ".sfx", "-v"};
        for (auto &p : split(param(lv_), ' '))
            if (!p.empty()) args.push_back(p);
        args.push_back("-" + ver);
        fs::create_directories(dir_ / "out" / u8path(ver));
        program("tr3pcpsx", args, {{"TR3PCPSX_MODULES", path_str(fs::absolute(job_.modules))}});
        fs::path psx = dir_ / "out" / u8path(ver) / u8path(lv_ + ".psx");
        stop_empty_samples(psx);
        fs::rename(psx, job_.work / "out" / u8path(ver) / u8path(lv_ + ".psx"));
    }

private:
    const ConvertJob &job_;
    const std::string &s_;
    std::string lv_;
    fs::path dir_;

    std::string tr2() const { return lv_ + ".tr2"; }

    std::string param(const std::string &level) const {
        for (auto &p : split(setting(s_, "convparams"), ';'))
            if (starts_with(p, level + ":")) return p.substr(level.size() + 1);
        throw Error("levels/convert.sh: no conversion parameters for " + level);
    }

    std::string program(const std::string &name, const std::vector<std::string> &args,
                        const std::map<std::string, std::string> &env = {}) {
        return run_tool_program(name, args, dir_, env);
    }
    void level_tool(const std::string &name, const std::vector<std::string> &args) { run_level_tool(name, args, dir_); }

    void shell_line(const std::string &line) {
        std::vector<std::string> argv = shell_words(line);
        if (argv.empty()) return;
        if (argv[0] == "py") {
            if (argv.size() < 3 || argv[1] != "-3") throw Error("unexpected command: " + line);
            std::string tool = argv[2].substr(argv[2].find_last_of("/\\") + 1);
            std::vector<std::string> rest;
            for (size_t i = 3; i < argv.size(); i++)
                if (argv[i] != "||" && argv[i] != "exit" && argv[i] != "1") rest.push_back(argv[i]);
            level_tool(tool, rest);
        } else {
            if (!starts_with(argv[0], "./")) throw Error("unexpected command: " + line);
            program(argv[0].substr(2), std::vector<std::string>(argv.begin() + 1, argv.end()));
        }
    }

    void sounds() {
        fs::path tmp = dir_ / "tmpsfx";
        fs::copy(job_.samples, tmp, fs::copy_options::recursive);
        for (auto &p : split(setting(s_, "nosfx"), ';')) {
            if (!starts_with(p, lv_ + ":")) continue;
            for (auto &sample : split(p.substr(lv_.size() + 1), ','))
                if (!sample.empty()) write_file(tmp / u8path(sample + ".wav"), data_file("prj/levels/sfx/empty.wav"));
        }
        fs::create_directories(dir_ / "outsfx" / u8path(job_.version));
        program("trsfx", {"./tmpsfx", "./outsfx/" + job_.version + "/" + lv_ + ".sfx"});
        fs::remove_all(tmp);
    }

    void textures() {
        program("3texex", {tr2(), "cur.bmp"});
        Bytes cur = read_file(dir_ / "cur.bmp");
        write_file(dir_ / "tex.bmp", apply_texture_patch(cur, data_file("own/assets/" + lv_ + ".tex")));
        program("3texrep", {tr2(), "tex.bmp"});
        fs::remove(dir_ / "cur.bmp");
        fs::remove(dir_ / "tex.bmp");
    }

    void anims() {
        program("troi", {tr2(), "e", "cur.dat"});
        Bytes cur = read_file(dir_ / "cur.dat");
        fs::remove(dir_ / "cur.dat");
        write_file(dir_ / "anim.dat", apply_bindiff(rebuild_anim_block(cur), data_file("own/assets/" + lv_ + ".anim")));
        program("troi", {tr2(), "i", "anim.dat"});
        fs::remove(dir_ / "anim.dat");
        for (auto *t : ANIM_TOOLS) level_tool(t, {tr2()});
    }

    void geom() {
        for (auto &line : split(data_text("prj/levels/geom/" + lv_ + ".sh"), '\n')) {
            std::string t = trim(line);
            if (starts_with(t, "./")) shell_line(t);
        }
    }

    void mesh() {
        RunResult r = run_program(program_path("tr3mesh"), {tr2()}, dir_, {}, true);
        if (r.exit_code != 0) throw Error("tr3mesh " + tr2() + ": exit " + std::to_string(r.exit_code));
        Bytes cur(r.out.begin(), r.out.end());
        write_file(dir_ / "mesh.txt", apply_bindiff(cur, data_file("own/assets/" + lv_ + ".mesh")));
        program("tr3mesh", {tr2(), "mesh.txt"});
        fs::remove(dir_ / "mesh.txt");
    }

    // The Japan/Asia Disc 1 level (<level>_jp.tr2).
    void japan() {
        if (job_.version != "jp" && job_.version != "as") return;
        std::string jp = lv_ + "_jp.tr2";
        fs::copy_file(dir_ / u8path(tr2()), dir_ / u8path(jp));
        if (lv_ == "scotland" || lv_ == "chunnel") level_tool("share_model.py", {jp, jp, "167:192"});
        if (lv_ == "chunnel")
            level_tool("restore_model.py", {path_str(fs::absolute(job_.inputs / "chunnel.tr2")), jp, jp, "162"});
        std::string name = "prj/levels/japan/" + lv_ + ".txt";
        if (!data_has(name)) return;
        std::vector<std::string> cmds;
        for (auto &line : split(data_text(name), '\n'))
            if (!trim(line).empty() && line[0] != '#') cmds.push_back(line);
        for (size_t i = 0; i < cmds.size(); i += 60) {
            std::vector<std::string> args{"tr3pc", jp};
            args.insert(args.end(), cmds.begin() + long(i), cmds.begin() + long(std::min(cmds.size(), i + 60)));
            program("trmod", args);
        }
    }
};

}

void convert_levels(const ConvertJob &job) {
    std::string script = data_text("prj/levels/convert.sh");
    std::vector<std::string> levels = split(setting(script, "levels"), ',');
    if (fs::exists(job.work)) fs::remove_all(job.work);
    fs::create_directories(job.work / "out" / u8path(job.version));
    job.log("converting " + std::to_string(levels.size()) + " levels side by side");

    std::atomic<size_t> next{0};
    std::mutex m;
    std::string error;
    auto worker = [&] {
        for (;;) {
            size_t i = next++;
            if (i >= levels.size()) return;
            {
                std::lock_guard<std::mutex> lock(m);
                if (!error.empty()) return;
            }
            try {
                LevelConverter(job, script, levels[i]).run();
                job.log("  " + levels[i] + " done");
            } catch (const std::exception &e) {
                std::lock_guard<std::mutex> lock(m);
                if (error.empty()) error = levels[i] + ": " + e.what();
            }
        }
    };
    unsigned n = std::max(1u, std::min<unsigned>(unsigned(levels.size()), std::thread::hardware_concurrency()));
    std::vector<std::thread> pool;
    for (unsigned i = 0; i < n; i++) pool.emplace_back(worker);
    for (auto &t : pool) t.join();
    if (!error.empty()) throw Error(error);
}
