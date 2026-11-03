#include "window.h"

#include "panel.h"

#include <shellapi.h>
#include <objidl.h>
#include <gdiplus.h>
#include <shobjidl.h>

#include <chrono>
#include <deque>
#include <memory>
#include <mutex>
#include <optional>
#include <regex>
#include <set>
#include <thread>

#include "../build/build.h"
#include "../build/inputs.h"
#include "panel.h"
#include "version.h"

namespace {

using namespace ui;

const wchar_t TITLE[] = L"Tomb Raider III: The Lost Artefact - PS1 Builder";
#define VERSION_STR2(x) #x
#define VERSION_STR(x) VERSION_STR2(x)
const char VERSION[] = VERSION_STR(BUILDER_VERSION_MAJOR) "." VERSION_STR(BUILDER_VERSION_MINOR);   // project(VERSION) 1.12.0: "1.12"
const char ISSUES[] = "https://github.com/Snake356/tr3-lost-artefact-psx/issues/new";
const UINT WM_APP_RESULT = WM_APP + 1, WM_APP_LOG = WM_APP + 2;
const UINT_PTR TIMER_TICK = 1;

// build steps as build_disc logs them: (log line, step name, typical seconds; for the bar and the time left)
struct StepInfo {
    const char *line;
    const wchar_t *name;
    double seconds;
};
const StepInfo STEPS[] = {{"disc files", L"Reading your PS1 disc", 13},
                          {"Lost Artefact PC files", L"Reading the PC files", 0.5},
                          {"converting the levels", L"Converting the levels", 35},
                          {"executable, texts, pictures", L"Executable, texts and pictures", 0.5},
                          {"writing the disc image", L"Writing the disc image", 3}};
const int NSTEPS = int(std::size(STEPS));

const std::pair<const char *, const wchar_t *> LEVEL_NAMES[] = {
    {"scotland", L"Highland Fling"}, {"willsden", L"Willard's Lair"}, {"chunnel", L"Shakespeare Cliff"},
    {"undersea", L"Sleeping with the Fishes"}, {"underse2", L"Sleeping with the Fishes (part 2)"},
    {"zoo", L"It's a Madhouse!"}, {"slinc", L"Reunion"}};

struct Credit {
    const wchar_t *role, *who, *note;
};
const Credit CREDITS[] = {
    {L"PS1 port", L"b122251", L""},
    {L"Carried the port on", L"FindingTheLost",
     L"Listed many of the bugs fixed since and came up with splitting Sleeping with the Fishes in two to stay within "
     L"the PS1's limits."},
    {L"Continued and finished by", L"XeNoN3560", L""},
    {L"Japanese/Asian Disc 1", L"XeNoN3560", L"The easier levels, adapted by hand as the original Japanese TR3 had them."},
    {L"PS1 disc builder", L"XeNoN3560", L""}};
const wchar_t ORIGIN[] = L"Continues b122251's original project: gitlab.com/b122251/tr3gpsx\n\n"
                         L"Claude (Anthropic) was used as an AI assistant to help finish the project.";
const wchar_t LEGAL[] = L"Tomb Raider III and The Lost Artefact (c) Core Design / Eidos Interactive; the rights are now "
                        L"held by Crystal Dynamics. This is a fan project and not affiliated with them. No game data is "
                        L"included: the builder needs your own copies of both games.";

std::wstring clock_text(double seconds) {
    int s = int(seconds + 0.5);
    wchar_t b[32];
    swprintf(b, 32, L"%d:%02d", s / 60, s % 60);
    return b;
}

fs::path temp_folder() {
    wchar_t b[MAX_PATH + 1];
    GetTempPathW(MAX_PATH, b);
    return fs::path(b) / (L"TR3LA_builder_" + std::to_wstring(GetCurrentProcessId()));
}

// What a recognition thread found.
struct Recognised {
    int which = 0;                       // 1 PS1 disc, 2 PC files, 3 US disc
    unsigned generation = 0;
    bool ok = false;
    std::wstring text;
    std::optional<Ps1Info> ps1;
    std::string releases;
};

// --- file dialogs ---------------------------------------------------------------------------------------------

std::wstring pick(HWND owner, bool save, bool folder, const wchar_t *title, const wchar_t *filter_name,
                  const wchar_t *filter, const std::wstring &initial = L"") {
    IFileDialog *d = nullptr;
    if (FAILED(CoCreateInstance(save ? CLSID_FileSaveDialog : CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER,
                                save ? IID_IFileSaveDialog : IID_IFileOpenDialog, reinterpret_cast<void **>(&d))))
        return {};
    DWORD opts = 0;
    d->GetOptions(&opts);
    d->SetOptions(opts | FOS_FORCEFILESYSTEM | (folder ? FOS_PICKFOLDERS : 0) | (save ? FOS_OVERWRITEPROMPT : FOS_FILEMUSTEXIST));
    d->SetTitle(title);
    if (!folder) {
        COMDLG_FILTERSPEC spec[2] = {{filter_name, filter}, {L"All files", L"*.*"}};
        d->SetFileTypes(2, spec);
    }
    if (save) d->SetDefaultExtension(L"bin");
    if (!initial.empty()) {
        fs::path p(initial);
        d->SetFileName(p.filename().c_str());
        IShellItem *dir = nullptr;
        if (p.has_parent_path() &&
            SUCCEEDED(SHCreateItemFromParsingName(p.parent_path().c_str(), nullptr, IID_PPV_ARGS(&dir)))) {
            d->SetFolder(dir);
            dir->Release();
        }
    }
    std::wstring out;
    if (SUCCEEDED(d->Show(owner))) {
        IShellItem *item = nullptr;
        if (SUCCEEDED(d->GetResult(&item))) {
            PWSTR s = nullptr;
            if (SUCCEEDED(item->GetDisplayName(SIGDN_FILESYSPATH, &s))) {
                out = s;
                CoTaskMemFree(s);
            }
            item->Release();
        }
    }
    d->Release();
    return out;
}

// --- the About window -----------------------------------------------------------------------------------------

class AboutWindow : public Panel {
public:
    explicit AboutWindow(HWND owner) : owner_(owner) {
        add(Kind::Button, L"Close", [this] { DestroyWindow(hwnd_); });
        create(L"TR3LAAbout", L"About", WS_POPUP | WS_CAPTION | WS_SYSMENU, owner, 10, 10);
        RECT c{0, 0, S(470), 10};
        layout(c.right);
        c.bottom = content_height_;
        AdjustWindowRectExForDpi(&c, WS_POPUP | WS_CAPTION | WS_SYSMENU, FALSE, 0, UINT(dpi_));
        RECT o;
        GetWindowRect(owner, &o);
        int w = c.right - c.left, h = c.bottom - c.top;
        SetWindowPos(hwnd_, nullptr, o.left + (o.right - o.left - w) / 2, o.top + S(60), w, h, SWP_NOZORDER);
        EnableWindow(owner, FALSE);
        ShowWindow(hwnd_, SW_SHOW);
    }

private:
    HWND owner_;
    std::vector<std::pair<RECT, int>> texts_;      // laid out text rows: rect, kind (0 role, 1 who, 2 note, 3 small_font)
    std::vector<std::wstring> strings_;

    void row(int &y, int x, int w, const std::wstring &s, int kind, int gap) {
        HFONT f = kind == 1 ? fonts_.bold : kind == 3 ? fonts_.small_font : fonts_.normal;
        int h = text_height(f, s, w);
        texts_.push_back({RECT{x, y, x + w, y + h}, kind});
        strings_.push_back(s);
        y += h + gap;
    }

    void layout(int width) override {
        texts_.clear();
        strings_.clear();
        int x = S(20), w = width - S(40), y = S(16) + text_size(fonts_.title, L"T").cy + S(20);
        for (auto &c : CREDITS) {
            row(y, x, w, c.role, 0, 0);
            row(y, x, w, c.who, 1, *c.note ? 0 : S(8));
            if (*c.note) row(y, x, w, c.note, 2, S(8));
        }
        y += S(6);
        row(y, x, w, ORIGIN, 3, S(6));
        row(y, x, w, LEGAL, 3, S(12));
        SIZE b = control_size(ctl(0));
        ctl(0).r = {width - S(20) - b.cx, y, width - S(20), y + b.cy};
        content_height_ = y + b.cy + S(16);
    }

    void paint(HDC dc, const RECT &c) override {
        fill(dc, c, CARD);
        int th = text_size(fonts_.title, L"T").cy;
        text(dc, L"Tomb Raider III: The Lost Artefact - PS1", {S(20), S(16), c.right - S(20), S(16) + th}, fonts_.title, HEAD);
        fill(dc, {S(20), S(16) + th + S(6), c.right - S(20), S(16) + th + S(8)}, ACCENT);
        for (size_t i = 0; i < texts_.size(); i++) {
            int k = texts_[i].second;
            text(dc, strings_[i], texts_[i].first, k == 1 ? fonts_.bold : k == 3 ? fonts_.small_font : fonts_.normal,
                 k == 0 || k == 3 ? MUTED : TEXT, DT_LEFT | DT_TOP | DT_WORDBREAK);
        }
    }

    LRESULT message(UINT msg, WPARAM wp, LPARAM, bool &handled) override {
        if (msg == WM_KEYDOWN && (wp == VK_ESCAPE || wp == VK_RETURN)) {
            DestroyWindow(hwnd_);
            return 0;
        }
        if (msg == WM_CLOSE) {
            DestroyWindow(hwnd_);
            return 0;
        }
        if (msg == WM_DESTROY) {
            EnableWindow(owner_, TRUE);
            SetForegroundWindow(owner_);
            return 0;
        }
        if (msg == WM_NCDESTROY) {
            SetWindowLongPtrW(hwnd_, GWLP_USERDATA, 0);
            delete this;
            return 0;
        }
        handled = false;
        return 0;
    }
};

// --- the builder window ---------------------------------------------------------------------------------------

class BuilderWindow : public Panel {
public:
    BuilderWindow() {
        b_report_ = add(Kind::Info, L"Report a problem", [this] { report(""); });
        b_about_ = add(Kind::Info, L"ⓘ  About", [this] { new AboutWindow(hwnd_); });
        b_ps1_ = add(Kind::Button, L"Choose...", [this] { pick_ps1(); });
        b_pc_image_ = add(Kind::Button, L"Disc image...", [this] { pick_pc(false); });
        b_pc_folder_ = add(Kind::Button, L"Folder...", [this] { pick_pc(true); });
        for (auto [label, value] : {std::pair{L"As the game has it", ""}, {L"Red", "red"}, {L"Purple", "purple"}}) {
            std::string v = value;
            seg_.push_back(add(Kind::Segment, label, [this, v] { set_blood(v); }));
        }
        ctl(seg_[0]).checked = true;
        c_test_ = add(Kind::Check, L"Level select on New Game (leave off for a normal playthrough)");
        c_weapons_ = add(Kind::Check, L"Stronger weapons as in the Japanese version (double damage; leave off for a "
                                      L"normal playthrough)");
        c_crystals_ = add(Kind::Check, L"Save crystals as in the Japanese version (a crystal heals Lara, saving does "
                                       L"not use one up; leave off for a normal playthrough)");
        c_ntsc_ = add(Kind::Check, L"NTSC 60 Hz (European discs, needs the US disc, any revision):", [this] { update_state(); });
        b_us_ = add(Kind::Button, L"Choose...", [this] { pick_us(); });
        b_out_ = add(Kind::Button, L"...", [this] { pick_out(); });
        b_build_ = add(Kind::Accent, L"▶  Build disc image", [this] { start(); });
        b_details_ = add(Kind::Link, L"▸ Details", [this] { toggle_details(!details_); });
        b_error_report_ = add(Kind::Link, L"Report this error on GitHub", [this] { report(last_error_); });
        ctl(b_error_report_).visible = false;

        create(L"TR3LABuilder", TITLE, WS_OVERLAPPEDWINDOW, nullptr, 10, 10);
        e_us_ = make_edit(false);
        e_out_ = make_edit(false);
        e_log_ = make_edit(true);
        SetWindowSubclass();
        ps1_info_text_ = L"Any version and revision; .bin/.cue, .iso or .zip";
        pc_info_text_ = L"Gold CD image or its installed game folder, or Tomb Raider I-III Remastered";
        step_text_ = L"Ready";
        RECT c{0, 0, S(860), 10};
        layout(c.right);
        c.bottom = content_height_;
        AdjustWindowRectExForDpi(&c, WS_OVERLAPPEDWINDOW, FALSE, 0, UINT(dpi_));
        SetWindowPos(hwnd_, nullptr, 0, 0, c.right - c.left, c.bottom - c.top, SWP_NOMOVE | SWP_NOZORDER);
        update_state();
        ShowWindow(hwnd_, SW_SHOW);
    }

public:
    void show_details_for_picture() {
        say("disc files");
        say("converting the levels");
        toggle_details(true);
    }

private:
    // controls
    int b_report_, b_about_, b_ps1_, b_pc_image_, b_pc_folder_, c_test_, c_weapons_, c_crystals_, c_ntsc_, b_us_, b_out_,
        b_build_, b_details_, b_error_report_;
    std::vector<int> seg_;
    HWND e_us_ = nullptr, e_out_ = nullptr, e_log_ = nullptr;
    bool details_ = false;

    // inputs and what was recognised
    std::wstring ps1_path_, pc_path_;
    std::optional<Ps1Info> ps1_info_, us_info_;
    bool pc_ok_ = false;
    std::string pc_releases_, blood_;
    std::wstring auto_out_;
    unsigned gen_[4] = {0, 0, 0, 0};
    int ps1_mark_ = 0, pc_mark_ = 0;               // 0 open, 1 ok, 2 bad, 3 checking
    std::wstring ps1_info_text_, pc_info_text_;

    // the build
    bool building_ = false;
    std::chrono::steady_clock::time_point t0_, step_t0_;
    int step_ = 0, levels_done_ = 0;
    double typical_[NSTEPS] = {};
    double progress_ = 0;
    std::wstring step_text_, detail_text_, time_text_, missing_text_;
    std::string last_error_;
    std::mutex queue_mutex_;
    std::deque<std::string> queue_;

    // layout results
    RECT head_{}, card1_{}, card2_{}, card3_{}, prog_{}, bar_{};
    RECT ps1_title_{}, ps1_info_r_{}, pc_title_{}, pc_info_r_{}, opt_title_{}, blood_label_{}, save_label_{}, missing_{},
        step_r_{}, detail_r_{}, time_r_{}, us_field_{}, out_field_{}, log_field_{};
    POINT ps1_mark_at_{}, pc_mark_at_{};

    // EDIT controls report changes to the out field through WM_COMMAND
    void SetWindowSubclass() {}

    // --- layout and painting ---

    void place_edit(HWND e, RECT field) {
        if (!e) return;
        int pad = S(5);
        MoveWindow(e, field.left + pad, field.top + pad, field.right - field.left - 2 * pad, field.bottom - field.top - 2 * pad, TRUE);
    }

    void layout(int width) override {
        if (width <= 0) return;
        int pad = S(16), cp = S(12), y = 0;
        int line = text_size(fonts_.normal, L"Ag").cy, bline = text_size(fonts_.bold, L"Ag").cy;
        int th = text_size(fonts_.title, L"Ag").cy, sh = text_size(fonts_.small_font, L"Ag").cy;

        head_ = {0, 0, width, S(10) + th + sh + S(10)};
        SIZE ab = control_size(ctl(b_about_)), rb = control_size(ctl(b_report_));
        int hy = (head_.bottom - ab.cy) / 2;
        ctl(b_about_).r = {width - pad - ab.cx, hy, width - pad, hy + ab.cy};
        ctl(b_report_).r = {ctl(b_about_).r.left - S(4) - rb.cx, hy, ctl(b_about_).r.left - S(4), hy + rb.cy};
        y = head_.bottom + S(2) + S(12);

        int left = pad, right = width - pad, mark_w = S(26);
        auto card = [&](RECT &cr, RECT &title, RECT &info_r, POINT &mark, const std::wstring &info_text,
                        std::vector<int> buttons) {
            int bx = right - cp;
            int bh = 0;
            for (auto it = buttons.rbegin(); it != buttons.rend(); ++it) {
                SIZE s = control_size(ctl(*it));
                ctl(*it).r = {bx - s.cx, 0, bx, s.cy};
                bx -= s.cx + S(6);
                bh = std::max<int>(bh, s.cy);
            }
            int tx = left + cp + mark_w, tw = bx - tx - S(8);
            int ih = std::max(line, text_height(fonts_.normal, info_text, tw));
            int h = std::max(cp + bline + ih + cp, bh + 2 * cp);
            cr = {left, y, right, y + h};
            title = {tx, y + cp, tx + tw, y + cp + bline};
            info_r = {tx, title.bottom, tx + tw, title.bottom + ih};
            mark = {left + cp, y + cp - S(2)};
            for (int b : buttons) OffsetRect(&ctl(b).r, 0, y + (h - bh) / 2);
            y += h + S(10);
        };
        card(card1_, ps1_title_, ps1_info_r_, ps1_mark_at_, ps1_info_text_, {b_ps1_});
        card(card2_, pc_title_, pc_info_r_, pc_mark_at_, pc_info_text_, {b_pc_image_, b_pc_folder_});

        // options
        int oy = y + cp, ox = left + cp;
        opt_title_ = {ox, oy, right - cp, oy + bline};
        oy += bline + S(6);
        int segh = control_size(ctl(seg_[0])).cy;
        SIZE bl = text_size(fonts_.normal, L"Blood");
        blood_label_ = {ox + S(26), oy, ox + S(26) + bl.cx, oy + segh};
        int sx = blood_label_.right + S(10);
        for (int s : seg_) {
            SIZE z = control_size(ctl(s));
            ctl(s).r = {sx, oy, sx + z.cx, oy + z.cy};
            sx += z.cx - 1;
        }
        oy += segh + S(8);
        for (int c : {c_test_, c_weapons_, c_crystals_}) {
            SIZE z = control_size(ctl(c));
            ctl(c).r = {ox + S(22), oy, std::min<int>(ox + S(22) + z.cx, right - cp), oy + z.cy};
            oy += z.cy + S(6);
        }
        int fh = line + S(10);
        SIZE nz = control_size(ctl(c_ntsc_)), uz = control_size(ctl(b_us_));
        ctl(c_ntsc_).r = {ox + S(22), oy, ox + S(22) + nz.cx, oy + fh};
        ctl(b_us_).r = {right - cp - uz.cx, oy + (fh - uz.cy) / 2, right - cp, oy + (fh - uz.cy) / 2 + uz.cy};
        us_field_ = {ctl(c_ntsc_).r.right + S(8), oy, ctl(b_us_).r.left - S(6), oy + fh};
        oy += fh + cp;
        card3_ = {left, y, right, oy};
        y = oy + S(12);

        // save as
        SIZE sl = text_size(fonts_.normal, L"Save as"), bz = control_size(ctl(b_build_)), dz = control_size(ctl(b_out_));
        int rowh = std::max<int>(bz.cy, fh);
        save_label_ = {left, y, left + sl.cx, y + rowh};
        ctl(b_build_).r = {right - bz.cx, y + (rowh - bz.cy) / 2, right, y + (rowh - bz.cy) / 2 + bz.cy};
        ctl(b_out_).r = {ctl(b_build_).r.left - S(10) - dz.cx - S(6), y + (rowh - dz.cy) / 2, ctl(b_build_).r.left - S(10),
                         y + (rowh - dz.cy) / 2 + dz.cy};
        out_field_ = {save_label_.right + S(8), y + (rowh - fh) / 2, ctl(b_out_).r.left - S(6), y + (rowh - fh) / 2 + fh};
        y += rowh + S(4);
        missing_ = {left, y, right, y + line};
        y += line + S(10);

        // progress
        int py = y + S(10);
        step_r_ = {left + cp, py, right - cp, py + line};
        detail_r_ = step_r_;
        py += line + S(6);
        bar_ = {left + cp, py, right - cp, py + S(8)};
        py += S(8) + S(6);
        SIZE lz = control_size(ctl(b_details_)), ez = control_size(ctl(b_error_report_));
        ctl(b_details_).r = {left + cp, py, left + cp + lz.cx, py + lz.cy};
        ctl(b_error_report_).r = {right - cp - ez.cx, py, right - cp, py + ez.cy};
        time_r_ = {left + cp, py, right - cp, py + lz.cy};
        py += lz.cy;
        if (details_) {
            log_field_ = {left + cp, py + S(6), right - cp, py + S(6) + S(190)};
            py = log_field_.bottom;
        }
        prog_ = {left, y, right, py + S(10)};
        content_height_ = prog_.bottom + pad;

        place_edit(e_us_, us_field_);
        place_edit(e_out_, out_field_);
        if (e_log_) {
            ShowWindow(e_log_, details_ ? SW_SHOW : SW_HIDE);
            if (details_) MoveWindow(e_log_, log_field_.left + S(4), log_field_.top + S(4), log_field_.right - log_field_.left - S(8),
                                     log_field_.bottom - log_field_.top - S(8), TRUE);
        }
    }

    void draw_mark(HDC dc, POINT at, int state) {
        const wchar_t *m = state == 1 ? L"✓" : state == 2 ? L"✗" : state == 3 ? L"…" : L"○";
        text(dc, m, {at.x, at.y, at.x + S(26), at.y + S(26)}, fonts_.mark, state == 1 ? OK : state == 2 ? BAD : MUTED,
             DT_LEFT | DT_TOP | DT_SINGLELINE);
    }

    void paint(HDC dc, const RECT &c) override {
        fill(dc, c, BG);
        fill(dc, head_, CARD);
        fill(dc, {0, head_.bottom, c.right, head_.bottom + S(2)}, ACCENT);
        int th = text_size(fonts_.title, L"Ag").cy;
        text(dc, L"Tomb Raider III: The Lost Artefact", {S(16), S(10), c.right, S(10) + th}, fonts_.title, HEAD);
        text(dc, L"PS1 disc builder", {S(16), S(10) + th, c.right, head_.bottom}, fonts_.small_font, MUTED);

        for (RECT *r : {&card1_, &card2_, &card3_, &prog_}) fill(dc, *r, CARD);
        draw_mark(dc, ps1_mark_at_, ps1_mark_);
        text(dc, L"1   Your Tomb Raider III PS1 disc", ps1_title_, fonts_.bold, TEXT);
        text(dc, ps1_info_text_, ps1_info_r_, fonts_.normal, ps1_mark_ == 1 ? OK : ps1_mark_ == 2 ? BAD : MUTED, DT_WORDBREAK);
        draw_mark(dc, pc_mark_at_, pc_mark_);
        text(dc, L"2   Lost Artefact PC files", pc_title_, fonts_.bold, TEXT);
        text(dc, pc_info_text_, pc_info_r_, fonts_.normal, pc_mark_ == 1 ? OK : pc_mark_ == 2 ? BAD : MUTED, DT_WORDBREAK);
        text(dc, L"3   Options", opt_title_, fonts_.bold, TEXT);
        text(dc, L"Blood", blood_label_, fonts_.normal, MUTED, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

        bool us_on = IsWindowEnabled(e_us_);
        fill(dc, us_field_, us_on ? BG : CARD);
        frame(dc, us_field_, LINE);
        text(dc, L"Save as", save_label_, fonts_.normal, TEXT, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        fill(dc, out_field_, IsWindowEnabled(e_out_) ? BG : CARD);
        frame(dc, out_field_, LINE);
        text(dc, missing_text_, missing_, fonts_.normal, BAD, DT_RIGHT | DT_TOP | DT_SINGLELINE | DT_END_ELLIPSIS);

        text(dc, step_text_, step_r_, fonts_.normal, MUTED, DT_LEFT | DT_TOP | DT_SINGLELINE);
        text(dc, detail_text_, detail_r_, fonts_.normal, MUTED, DT_RIGHT | DT_TOP | DT_SINGLELINE);
        fill(dc, bar_, LINE);
        RECT done = bar_;
        done.right = bar_.left + LONG((bar_.right - bar_.left) * std::min(progress_, 100.0) / 100.0);
        fill(dc, done, ACCENT);
        if (!ctl(b_error_report_).visible) text(dc, time_text_, time_r_, fonts_.normal, MUTED, DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
        if (details_) fill(dc, log_field_, BG);
    }

    LRESULT message(UINT msg, WPARAM wp, LPARAM lp, bool &handled) override {
        switch (msg) {
        case WM_GETMINMAXINFO: {
            auto *mm = reinterpret_cast<MINMAXINFO *>(lp);
            RECT r{0, 0, S(760), content_height_ ? content_height_ : S(600)};
            AdjustWindowRectExForDpi(&r, WS_OVERLAPPEDWINDOW, FALSE, 0, UINT(dpi_));
            mm->ptMinTrackSize = {r.right - r.left, r.bottom - r.top};
            mm->ptMaxTrackSize.y = r.bottom - r.top;
            return 0;
        }
        case WM_COMMAND:
            if (HWND(lp) == e_out_ && HIWORD(wp) == EN_CHANGE) update_ready();
            if (HWND(lp) == e_us_ && HIWORD(wp) == EN_KILLFOCUS) recognise_us();
            return 0;
        case WM_APP_RESULT: {
            std::unique_ptr<Recognised> r(reinterpret_cast<Recognised *>(lp));
            if (r->generation == gen_[r->which]) recognised(*r);
            return 0;
        }
        case WM_APP_LOG:
            drain();
            return 0;
        case WM_TIMER:
            if (wp == TIMER_TICK && building_) tick();
            return 0;
        case WM_CLOSE:
            if (building_ && MessageBoxW(hwnd_, L"A disc image is being built. Stop and close the builder?", TITLE,
                                         MB_YESNO | MB_ICONWARNING) != IDYES)
                return 0;
            DestroyWindow(hwnd_);
            return 0;
        case WM_DESTROY: {
            std::error_code ec;
            fs::remove_all(temp_folder(), ec);
            PostQuitMessage(0);
            return 0;
        }
        }
        handled = false;
        return 0;
    }

    void fit_height() {
        RECT c;
        GetClientRect(hwnd_, &c);
        layout(c.right);
        RECT w{0, 0, c.right, content_height_};
        AdjustWindowRectExForDpi(&w, WS_OVERLAPPEDWINDOW, FALSE, 0, UINT(dpi_));
        RECT cur;
        GetWindowRect(hwnd_, &cur);
        if (cur.bottom - cur.top != w.bottom - w.top)
            SetWindowPos(hwnd_, nullptr, 0, 0, cur.right - cur.left, w.bottom - w.top, SWP_NOMOVE | SWP_NOZORDER);
        redraw();
    }

    // --- inputs ---

    std::wstring edit_text(HWND e) const {
        int n = GetWindowTextLengthW(e);
        std::wstring s(size_t(n) + 1, L'\0');
        GetWindowTextW(e, s.data(), n + 1);
        s.resize(size_t(n));
        return s;
    }

    void pick_ps1() {
        std::wstring p = pick(hwnd_, false, false, L"Tomb Raider III PS1 disc image", L"Disc images", L"*.cue;*.bin;*.iso;*.zip");
        if (p.empty()) return;
        ps1_path_ = p;
        recognise(1, p);
    }

    void pick_us() {
        std::wstring p = pick(hwnd_, false, false, L"Tomb Raider III PS1 disc image (USA)", L"Disc images", L"*.cue;*.bin;*.iso;*.zip");
        if (p.empty()) return;
        SetWindowTextW(e_us_, p.c_str());
        recognise_us();
    }

    void pick_pc(bool folder) {
        std::wstring p = folder ? pick(hwnd_, false, true, L"Lost Artefact game folder (or Tomb Raider I-III Remastered)", L"", L"")
                                : pick(hwnd_, false, false, L"Lost Artefact PC disc image", L"Disc images", L"*.iso;*.cue;*.bin;*.zip");
        if (p.empty()) return;
        pc_path_ = p;
        recognise(2, p);
    }

    void pick_out() {
        std::wstring p = pick(hwnd_, true, false, L"Save disc image", L"Disc image", L"*.bin", edit_text(e_out_));
        if (!p.empty()) SetWindowTextW(e_out_, p.c_str());
    }

    void recognise_us() {
        us_info_.reset();
        std::wstring p = edit_text(e_us_);
        if (p.empty()) {
            update_state();
            return;
        }
        recognise(3, p);
    }

    // Recognition runs in the background (a zipped image is unpacked first); the newest request wins.
    void recognise(int which, const std::wstring &path) {
        unsigned g = ++gen_[which];
        if (which == 1) {
            ps1_info_.reset();
            ps1_mark_ = 3;
            ps1_info_text_ = L"Checking...";
        } else if (which == 2) {
            pc_ok_ = false;
            pc_mark_ = 3;
            pc_info_text_ = L"Checking...";
        }
        update_state();
        fit_height();
        HWND h = hwnd_;
        std::thread([h, which, g, path] {
            auto r = std::make_unique<Recognised>();
            r->which = which;
            r->generation = g;
            fs::path tmp = temp_folder() / ("recognise" + std::to_string(which) + "_" + std::to_string(g));
            try {
                fs::path p(path);
                if (which == 2 && fs::is_directory(p)) {
                    describe_pc(*r, pc_files_from_folder(p));
                } else {
                    auto disc = open_image(p, tmp);
                    if (which == 2) {
                        describe_pc(*r, pc_files_from_disc(*disc));
                    } else {
                        auto info = recognise_ps1(*disc);
                        if (!info) {
                            r->text = L"Not a known Tomb Raider III PS1 disc.";
                        } else if (!info->supported) {
                            r->text = widen(info->language + ", " + info->revision + ": this revision is not supported yet.");
                        } else {
                            r->ok = true;
                            r->ps1 = info;
                            r->text = widen("Tomb Raider III " + info->language + ", " + info->revision + " - recognised");
                        }
                    }
                }
            } catch (const std::exception &e) {
                r->ok = false;
                r->text = widen(e.what());
            }
            std::error_code ec;
            fs::remove_all(tmp, ec);
            if (!PostMessageW(h, WM_APP_RESULT, 0, LPARAM(r.get()))) return;
            r.release();
        }).detach();
    }

    static void describe_pc(Recognised &r, const PcFiles &files) {
        auto res = recognise_pc(files);
        std::string missing, unknown;
        std::set<std::string> rels;
        auto order = std::vector<std::string>(std::begin(PC_LEVELS), std::end(PC_LEVELS));
        order.push_back("main.sfx");
        for (auto &n : order) {
            const PcRecognised &x = res[n];
            if (!x.present) missing += (missing.empty() ? "" : ", ") + n;
            else if (x.release.empty()) unknown += (unknown.empty() ? "" : ", ") + n;
            else rels.insert(x.release);
        }
        if (!missing.empty() || !unknown.empty()) {
            std::string t;
            if (!missing.empty()) {
                t = "missing: " + missing;
                if (missing.find("main.sfx") != std::string::npos) t += " (MAIN.SFX holds the sounds, it is next to the level folder)";
            }
            if (!unknown.empty()) t += (t.empty() ? "" : "; ") + std::string("unknown version: ") + unknown;
            r.text = widen(t);
            return;
        }
        for (auto &x : rels) r.releases += (r.releases.empty() ? "" : ", ") + x;
        r.ok = true;
        r.text = widen("Lost Artefact found (" + r.releases + ") - recognised");
    }

    void recognised(const Recognised &r) {
        if (r.which == 1) {
            ps1_info_ = r.ok ? r.ps1 : std::nullopt;
            ps1_mark_ = r.ok ? 1 : 2;
            ps1_info_text_ = r.text;
        } else if (r.which == 2) {
            pc_ok_ = r.ok;
            pc_mark_ = r.ok ? 1 : 2;
            pc_info_text_ = r.text;
            if (r.ok) pc_releases_ = r.releases;
        } else {
            us_info_ = r.ps1 && (r.ps1->version == "us" || r.ps1->version == "us10") ? r.ps1 : std::nullopt;
        }
        update_state();
        fit_height();
    }

    void set_blood(const std::string &v) {
        blood_ = v;
        for (size_t i = 0; i < seg_.size(); i++) ctl(seg_[i]).checked = (i == 0 && v.empty()) || (i == 1 && v == "red") || (i == 2 && v == "purple");
        redraw();
    }

    // which options this disc allows
    void update_state() {
        if (building_) return;
        bool european = ps1_info_ && ntsc_possible(ps1_info_->version);
        if (!european) ctl(c_ntsc_).checked = false;
        ctl(c_ntsc_).enabled = european;
        bool jp1 = ps1_info_ && (ps1_info_->version == "jp" || ps1_info_->version == "as");
        if (jp1) ctl(c_weapons_).checked = ctl(c_crystals_).checked = true;   // the Japanese/Asian Disc 1 have them
        ctl(c_weapons_).enabled = ctl(c_crystals_).enabled = !jp1;
        bool ntsc = ctl(c_ntsc_).checked;
        EnableWindow(e_us_, ntsc);
        ctl(b_us_).enabled = ntsc;
        suggest_name();
        update_ready();
        redraw();
    }

    std::vector<std::wstring> missing() const {
        std::vector<std::wstring> out;
        if (!ps1_info_) out.push_back(ps1_path_.empty() ? L"1: choose your PS1 disc" : L"1: PS1 disc not recognised");
        if (!pc_ok_) out.push_back(pc_path_.empty() ? L"2: choose the PC disc image or folder" : L"2: PC files not recognised");
        if (ctl_checked(c_ntsc_) && !us_info_)
            out.push_back(edit_text(e_us_).empty() ? L"60 Hz: choose the US disc" : L"60 Hz: US disc not recognised");
        std::wstring o = edit_text(e_out_);
        if (o.find_first_not_of(L" \t") == std::wstring::npos) out.push_back(L"choose where to save the disc image");
        return out;
    }
    bool ctl_checked(int i) const { return controls_[size_t(i)].checked; }

    void update_ready() {
        if (building_) return;
        auto m = missing();
        ctl(b_build_).enabled = m.empty();
        missing_text_.clear();
        for (auto &x : m) missing_text_ += (missing_text_.empty() ? L"Still missing: " : L"; ") + x;
        redraw();
    }

    // The image named after the disc it is built from, as the dump names it: "Tomb Raider III - The Lost Artefact
    // (Germany) (Rev 1)", "(Rev 0)" for a first release, "(NTSC USA Rev 0)" for 60 Hz with the US disc's revision
    // (its executable runs the game). A name typed by the player stays; a folder chosen for the suggested name stays.
    void suggest_name() {
        std::wstring cur = edit_text(e_out_);
        if (!ps1_info_ || (!cur.empty() && fs::path(cur).filename() != fs::path(auto_out_).filename())) return;
        std::string r = ps1_info_->redump;
        size_t dash = r.find(" - ");
        std::string tags = dash == std::string::npos ? r : r.substr(dash + 3);
        tags = tags.substr(std::min(tags.size(), tags.find('(')));
        for (size_t p; (p = tags.find(" (EDC)")) != std::string::npos;) tags.erase(p, 6);
        if (tags.find("(Rev ") == std::string::npos && tags.find("(Disc ") == std::string::npos) tags += " (Rev 0)";
        if (ctl_checked(c_ntsc_)) {
            if (us_info_) {
                size_t p = us_info_->redump.find("(Rev ");
                tags += " (NTSC USA Rev " + (p != std::string::npos ? us_info_->redump.substr(p + 5, 1) : std::string("0")) + ")";
            } else {
                tags += " (NTSC)";
            }
        }
        std::string title = starts_with(tags, "(USA)") ? "The Lost Artifact" : "The Lost Artefact";
        fs::path folder = fs::path(cur.empty() ? ps1_path_ : cur).parent_path();
        auto_out_ = (folder / widen("Tomb Raider III - " + title + " " + tags + ".bin")).wstring();
        SetWindowTextW(e_out_, auto_out_.c_str());
    }

    // --- problem reports ---

    std::string without_paths(std::string text) const {
        std::vector<std::wstring> known = {ps1_path_, pc_path_, edit_text(e_us_), edit_text(e_out_)};
        wchar_t home[MAX_PATH] = L"";
        if (GetEnvironmentVariableW(L"USERPROFILE", home, MAX_PATH)) known.push_back(home);
        size_t n = known.size();
        for (size_t i = 0; i < n; i++)
            if (!known[i].empty()) known.push_back(fs::path(known[i]).parent_path().wstring());
        std::vector<std::string> forms;
        for (auto &k : known) {
            if (k.empty()) continue;
            std::string s = narrow(fs::absolute(fs::path(k)).wstring());
            forms.push_back(s);
            std::string f = s;
            for (auto &c : f) if (c == '\\') c = '/';
            forms.push_back(f);
        }
        std::sort(forms.begin(), forms.end(), [](auto &a, auto &b) { return a.size() > b.size(); });
        for (auto &f : forms)
            for (size_t p; !f.empty() && (p = text.find(f)) != std::string::npos;) text.replace(p, f.size(), "<path>");
        // any other path, up to the next separator (spaces included: rather too much than a folder name)
        return std::regex_replace(text, std::regex(R"((?:[A-Za-z]:)?[\\/][^\n:;,'"]*[\\/][^\n:;,'"]*)"), "<path>");
    }

    static std::string url_quote(const std::string &s) {
        std::string out;
        for (unsigned char c : s) {
            if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') out += char(c);
            else if (c == ' ') out += '+';
            else {
                char b[4];
                snprintf(b, sizeof b, "%%%02X", c);
                out += b;
            }
        }
        return out;
    }

    void report(const std::string &error) {
        OSVERSIONINFOW v{sizeof v};
        using RtlGetVersion = LONG(WINAPI *)(OSVERSIONINFOW *);
        if (auto f = reinterpret_cast<RtlGetVersion>(GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "RtlGetVersion"))) f(&v);
        auto on = [](bool b) { return b ? "on" : "off"; };
        std::vector<std::string> lines = {
            std::string("Builder ") + VERSION + " (C++), Windows " + std::to_string(v.dwMajorVersion) + "." +
                std::to_string(v.dwMinorVersion) + "." + std::to_string(v.dwBuildNumber),
            "PS1 disc: " + (ps1_info_ ? ps1_info_->redump : std::string("not chosen or not recognised")),
            "PC files: " + (pc_ok_ ? pc_releases_ : std::string("not chosen or not recognised")),
            std::string("Options: blood ") + (blood_.empty() ? "default" : blood_) + ", level select " + on(ctl_checked(c_test_)) +
                ", NTSC " + on(ctl_checked(c_ntsc_)) + ", Japanese weapons " + on(ctl_checked(c_weapons_)) +
                ", Japanese crystals " + on(ctl_checked(c_crystals_))};
        std::string title, body;
        if (!error.empty()) {
            std::string msg = without_paths(error).substr(0, 1200);
            title = "Builder error: " + msg.substr(0, std::min<size_t>(msg.find('\n'), 80));
            body = "**What happened**\nThe build stopped with this error:\n```\n" + msg + "\n```\n\n";
        } else {
            body = "**What happened**\n(For a problem in the game: which disc version, which level, where - room number "
                   "from TRView if you can - and what you did just before.)\n\n";
        }
        body += "**Setup**\n";
        for (auto &l : lines) body += "- " + l + "\n";
        std::string url = std::string(ISSUES) + "?title=" + url_quote(title) + "&body=" + url_quote(body);
        ShellExecuteW(hwnd_, L"open", widen(url).c_str(), nullptr, nullptr, SW_SHOWNORMAL);
    }

    // --- building ---

    void toggle_details(bool show) {
        details_ = show;
        ctl(b_details_).text = std::wstring(show ? L"▾" : L"▸") + L" Details";
        fit_height();
    }

    void say(const std::string &line) {
        std::wstring w = widen(line);
        std::wstring t;
        for (wchar_t c : w) {
            if (c == L'\n') t += L"\r\n";
            else t += c;
        }
        t += L"\r\n";
        int n = GetWindowTextLengthW(e_log_);
        SendMessageW(e_log_, EM_SETSEL, n, n);
        SendMessageW(e_log_, EM_REPLACESEL, FALSE, LPARAM(t.c_str()));
    }

    void follow(const std::string &m) {
        for (int k = 0; k < NSTEPS; k++)
            if (starts_with(m, STEPS[k].line)) {
                step_ = k;
                step_t0_ = std::chrono::steady_clock::now();
                levels_done_ = 0;
                step_text_ = L"Step " + std::to_wstring(k + 1) + L" of " + std::to_wstring(NSTEPS) + L": " + STEPS[k].name;
                detail_text_.clear();
                return;
            }
        std::string t = trim(m);
        if (ends_with(t, " done"))
            for (auto &[lv, name] : LEVEL_NAMES)
                if (t == std::string(lv) + " done") {
                    levels_done_++;
                    detail_text_ = std::wstring(name) + L" (" + std::to_wstring(levels_done_) + L"/" +
                                   std::to_wstring(std::size(LEVEL_NAMES)) + L")";
                }
    }

    // The bar moves on smoothly within a step (by its typical time, at most to 97 % of the step; by the finished
    // levels while converting). The time left: the rest of the step and the later steps by their typical times,
    // scaled by how long the finished steps took on this PC.
    void tick() {
        using sec = std::chrono::duration<double>;
        auto now = std::chrono::steady_clock::now();
        int k = step_;
        double before = 0, after = 0, total = 0;
        for (int i = 0; i < NSTEPS; i++) {
            total += typical_[i];
            if (i < k) before += typical_[i];
            if (i > k) after += typical_[i];
        }
        double speed = k ? std::clamp(sec(step_t0_ - t0_).count() / before, 0.5, 4.0) : 1.0;
        double in_step = sec(now - step_t0_).count();
        double part = std::min(in_step / (typical_[k] * speed), 0.97);
        if (k == 2) part = std::max(part, double(levels_done_) / double(std::size(LEVEL_NAMES)));
        progress_ = std::max(progress_, 100 * (before + part * typical_[k]) / total);
        double left = std::max(typical_[k] * speed - in_step, 1.0) + speed * after;
        time_text_ = clock_text(sec(now - t0_).count()) + L" - about " + clock_text(left) + L" left";
        redraw();
    }

    void lock(bool locked) {
        for (int c : {b_ps1_, b_pc_image_, b_pc_folder_, c_test_, c_weapons_, c_crystals_, c_ntsc_, b_us_, b_out_})
            ctl(c).enabled = !locked;
        for (int s : seg_) ctl(s).enabled = !locked;
        EnableWindow(e_out_, !locked);
        EnableWindow(e_us_, !locked);
        if (!locked) {
            building_ = false;
            update_state();                         // the options that stay off for this disc
        }
        redraw();
    }

    void start() {
        if (!missing().empty()) return update_ready();
        BuildOptions o;
        o.ps1_image = ps1_path_;
        o.pc_source = pc_path_;
        o.out_bin = edit_text(e_out_);
        if (ctl_checked(c_ntsc_)) o.ntsc_image = edit_text(e_us_);
        o.blood = blood_;
        o.level_select = ctl_checked(c_test_);
        o.japan_weapons = ctl_checked(c_weapons_);
        o.japan_crystals = ctl_checked(c_crystals_);
        if (lower(path_str(o.out_bin.extension())) != ".bin") o.out_bin += ".bin";

        ctl(b_build_).enabled = false;
        lock(true);
        building_ = true;
        ctl(b_error_report_).visible = false;
        progress_ = 0;
        step_ = levels_done_ = 0;
        t0_ = step_t0_ = std::chrono::steady_clock::now();
        for (int i = 0; i < NSTEPS; i++) typical_[i] = STEPS[i].seconds;
        if (ctl_checked(c_ntsc_)) typical_[0] += 17;
        if (ps1_info_ && (ps1_info_->version == "jp" || ps1_info_->version == "as")) typical_[2] += 8;
        step_text_ = std::wstring(L"Step 1 of ") + std::to_wstring(NSTEPS) + L": " + STEPS[0].name;
        detail_text_.clear();
        missing_text_.clear();
        SetTimer(hwnd_, TIMER_TICK, 100, nullptr);
        HWND h = hwnd_;
        std::thread([this, h, o] {
            auto post = [this, h](const std::string &s) {
                {
                    std::lock_guard<std::mutex> lock(queue_mutex_);
                    queue_.push_back(s);
                }
                PostMessageW(h, WM_APP_LOG, 0, 0);
            };
            try {
                build_disc(o, post);
                post("\x01");
            } catch (const std::exception &e) {   // shown to the player; the work folder stays for checking
                post(std::string("\x02") + e.what() +
                     "\n(The intermediate files were kept in the folder build_work next to the disc image.)");
            }
        }).detach();
        redraw();
    }

    void drain() {
        std::deque<std::string> lines;
        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            lines.swap(queue_);
        }
        for (auto &m : lines) {
            if (!m.empty() && (m[0] == '\x01' || m[0] == '\x02')) {
                using sec = std::chrono::duration<double>;
                std::wstring took = clock_text(sec(std::chrono::steady_clock::now() - t0_).count());
                KillTimer(hwnd_, TIMER_TICK);
                time_text_.clear();
                lock(false);
                if (m[0] == '\x01') {
                    progress_ = 100;
                    step_text_ = L"Finished in " + took + L" - the disc image is ready.";
                    detail_text_.clear();
                    say("Finished in " + narrow(took) + ".");
                } else {
                    last_error_ = m.substr(1);
                    step_text_ = L"The build stopped - see the details.";
                    say("Error: " + last_error_);
                    ctl(b_error_report_).visible = true;
                    toggle_details(true);
                }
            } else {
                say(m);
                follow(m);
            }
        }
        redraw();
    }
};

}

int run_builder_window() {
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    new BuilderWindow();       // lives until the program ends
    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
        if (!IsDialogMessageW(GetActiveWindow(), &msg)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }
    CoUninitialize();
    return int(msg.wParam);
}

int save_window_picture(const wchar_t *png, bool details) {
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    Gdiplus::GdiplusStartupInput gi;
    ULONG_PTR token;
    Gdiplus::GdiplusStartup(&token, &gi, nullptr);
    auto *w = new BuilderWindow();
    if (details) w->show_details_for_picture();
    MSG msg;
    while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) DispatchMessageW(&msg);
    RECT r;
    GetWindowRect(w->hwnd(), &r);
    HDC screen = GetDC(nullptr), mem = CreateCompatibleDC(screen);
    HBITMAP bmp = CreateCompatibleBitmap(screen, r.right - r.left, r.bottom - r.top);
    SelectObject(mem, bmp);
    PrintWindow(w->hwnd(), mem, PW_RENDERFULLCONTENT);
    CLSID png_clsid;
    UINT n = 0, size = 0;
    Gdiplus::GetImageEncodersSize(&n, &size);
    std::vector<uint8_t> buf(size);
    auto *codecs = reinterpret_cast<Gdiplus::ImageCodecInfo *>(buf.data());
    Gdiplus::GetImageEncoders(n, size, codecs);
    for (UINT i = 0; i < n; i++)
        if (wcscmp(codecs[i].MimeType, L"image/png") == 0) png_clsid = codecs[i].Clsid;
    {
        Gdiplus::Bitmap b(bmp, nullptr);
        b.Save(png, &png_clsid, nullptr);
    }
    DeleteObject(bmp);
    DeleteDC(mem);
    ReleaseDC(nullptr, screen);
    DestroyWindow(w->hwnd());
    while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) DispatchMessageW(&msg);
    Gdiplus::GdiplusShutdown(token);
    return 0;
}
