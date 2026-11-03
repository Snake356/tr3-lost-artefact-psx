// A window painted by the builder itself in the dark style: buttons, check boxes, segment buttons and texts as
// light-weight controls (hit-tested and drawn here), edit fields as real EDIT controls in the same colours.
#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <functional>
#include <string>
#include <vector>

namespace ui {

extern const COLORREF BG, CARD, LINE, TEXT, MUTED, ACCENT, ACCENT_HOVER, ON_ACCENT, OK, BAD, HEAD, BUTTON_BORDER;

enum class Kind { Button, Accent, Info, Link, Check, Segment };

struct Control {
    Kind kind = Kind::Button;
    std::wstring text;
    RECT r{};
    bool enabled = true, visible = true, checked = false;
    std::function<void()> click;
};

struct Fonts {
    HFONT normal = nullptr, bold = nullptr, small_font = nullptr, title = nullptr, info = nullptr, mono = nullptr, mark = nullptr;
    void make(int dpi);
    void free();
};

class Panel {
public:
    virtual ~Panel();
    HWND hwnd() const { return hwnd_; }
    int S(int x) const { return MulDiv(x, dpi_, 96); }       // 96-dpi pixels to this window's pixels

protected:
    HWND hwnd_ = nullptr;
    int dpi_ = 96;
    Fonts fonts_;
    std::vector<Control> controls_;
    int hover_ = -1, pressed_ = -1;
    HBRUSH field_brush_ = nullptr, card_brush_ = nullptr;

    bool create(const wchar_t *cls, const wchar_t *title, DWORD style, HWND owner, int w, int h);
    int add(Kind kind, const std::wstring &text, std::function<void()> click = {});
    Control &ctl(int i) { return controls_[size_t(i)]; }

    // the window's own content
    virtual void layout(int width) = 0;                     // places the controls, sets content_height_
    virtual void paint(HDC dc, const RECT &client) = 0;     // backgrounds and texts (controls are drawn after)
    virtual LRESULT message(UINT msg, WPARAM wp, LPARAM lp, bool &handled);
    int content_height_ = 0;

    // helpers for painting and measuring
    SIZE text_size(HFONT f, const std::wstring &s) const;
    int text_height(HFONT f, const std::wstring &s, int width) const;            // wrapped
    void text(HDC dc, const std::wstring &s, RECT r, HFONT f, COLORREF c, UINT flags = DT_LEFT | DT_TOP | DT_NOPREFIX) const;
    void fill(HDC dc, const RECT &r, COLORREF c) const;
    void frame(HDC dc, const RECT &r, COLORREF c) const;
    SIZE control_size(const Control &c) const;
    HWND make_edit(bool multiline);
    void relayout();
    void redraw() { InvalidateRect(hwnd_, nullptr, FALSE); }

private:
    static LRESULT CALLBACK proc(HWND h, UINT msg, WPARAM wp, LPARAM lp);
    LRESULT handle(UINT msg, WPARAM wp, LPARAM lp);
    void draw_control(HDC dc, int i) const;
    int hit(POINT p) const;
    void set_dpi(int dpi);
};

std::wstring widen(const std::string &s);
std::string narrow(const std::wstring &w);

}
