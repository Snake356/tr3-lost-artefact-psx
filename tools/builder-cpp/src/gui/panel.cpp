#include "panel.h"

#include <dwmapi.h>
#include <uxtheme.h>
#include <windowsx.h>

namespace ui {

const COLORREF BG = RGB(0x1b, 0x1d, 0x22), CARD = RGB(0x24, 0x27, 0x2d), LINE = RGB(0x3a, 0x3d, 0x44),
               TEXT = RGB(0xe6, 0xe2, 0xd8), MUTED = RGB(0x9a, 0x95, 0x8a), ACCENT = RGB(0x4a, 0x90, 0xd9),
               ACCENT_HOVER = RGB(0x5e, 0xa0, 0xe6), ON_ACCENT = RGB(0x0f, 0x1a, 0x26), OK = RGB(0xa9, 0xd7, 0x9a),
               BAD = RGB(0xe8, 0x8a, 0x84), HEAD = RGB(0xf1, 0xea, 0xd6), BUTTON_BORDER = RGB(0x55, 0x5a, 0x62);

std::wstring widen(const std::string &s) {
    if (s.empty()) return {};
    int n = MultiByteToWideChar(CP_UTF8, 0, s.data(), int(s.size()), nullptr, 0);
    std::wstring w(size_t(n), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.data(), int(s.size()), w.data(), n);
    return w;
}

std::string narrow(const std::wstring &w) {
    if (w.empty()) return {};
    int n = WideCharToMultiByte(CP_UTF8, 0, w.data(), int(w.size()), nullptr, 0, nullptr, nullptr);
    std::string s(size_t(n), '\0');
    WideCharToMultiByte(CP_UTF8, 0, w.data(), int(w.size()), s.data(), n, nullptr, nullptr);
    return s;
}

namespace {
HFONT font(int dpi, int points, int weight, const wchar_t *face = L"Segoe UI") {
    return CreateFontW(-MulDiv(points, dpi, 72), 0, 0, 0, weight, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                       CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, face);
}
}

void Fonts::make(int dpi) {
    free();
    normal = font(dpi, 10, FW_NORMAL);
    bold = font(dpi, 10, FW_BOLD);
    small_font = font(dpi, 9, FW_NORMAL);
    title = font(dpi, 14, FW_BOLD);
    info = font(dpi, 11, FW_BOLD);
    mono = font(dpi, 9, FW_NORMAL, L"Consolas");
    mark = font(dpi, 13, FW_NORMAL, L"Segoe UI Symbol");
}

void Fonts::free() {
    for (HFONT *f : {&normal, &bold, &small_font, &title, &info, &mono, &mark})
        if (*f) DeleteObject(*f), *f = nullptr;
}

Panel::~Panel() {
    fonts_.free();
    if (field_brush_) DeleteObject(field_brush_);
    if (card_brush_) DeleteObject(card_brush_);
}

bool Panel::create(const wchar_t *cls, const wchar_t *title, DWORD style, HWND owner, int w, int h) {
    WNDCLASSEXW wc{sizeof wc};
    HINSTANCE inst = GetModuleHandleW(nullptr);
    if (!GetClassInfoExW(inst, cls, &wc)) {
        wc = {sizeof wc};
        wc.lpfnWndProc = proc;
        wc.hInstance = inst;
        wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
        wc.hIcon = LoadIconW(inst, MAKEINTRESOURCEW(1));
        wc.hIconSm = wc.hIcon;
        wc.lpszClassName = cls;
        RegisterClassExW(&wc);
    }
    field_brush_ = CreateSolidBrush(BG);
    card_brush_ = CreateSolidBrush(CARD);
    hwnd_ = CreateWindowExW(0, cls, title, style, CW_USEDEFAULT, CW_USEDEFAULT, w, h, owner, nullptr, inst, this);
    if (!hwnd_) return false;
    BOOL dark = TRUE;                                       // dark title bar (Windows 10 2004 and later)
    DwmSetWindowAttribute(hwnd_, 20 /* DWMWA_USE_IMMERSIVE_DARK_MODE */, &dark, sizeof dark);
    return true;
}

int Panel::add(Kind kind, const std::wstring &text, std::function<void()> click) {
    Control c;
    c.kind = kind;
    c.text = text;
    c.click = std::move(click);
    controls_.push_back(std::move(c));
    return int(controls_.size()) - 1;
}

SIZE Panel::text_size(HFONT f, const std::wstring &s) const {
    HDC dc = GetDC(hwnd_);
    HGDIOBJ old = SelectObject(dc, f);
    RECT r{0, 0, 0, 0};
    DrawTextW(dc, s.c_str(), int(s.size()), &r, DT_CALCRECT | DT_SINGLELINE | DT_NOPREFIX);
    SelectObject(dc, old);
    ReleaseDC(hwnd_, dc);
    return {r.right, r.bottom};
}

int Panel::text_height(HFONT f, const std::wstring &s, int width) const {
    if (s.empty()) return 0;
    HDC dc = GetDC(hwnd_);
    HGDIOBJ old = SelectObject(dc, f);
    RECT r{0, 0, width, 0};
    DrawTextW(dc, s.c_str(), int(s.size()), &r, DT_CALCRECT | DT_WORDBREAK | DT_NOPREFIX);
    SelectObject(dc, old);
    ReleaseDC(hwnd_, dc);
    return r.bottom;
}

void Panel::text(HDC dc, const std::wstring &s, RECT r, HFONT f, COLORREF c, UINT flags) const {
    SelectObject(dc, f);
    SetTextColor(dc, c);
    SetBkMode(dc, TRANSPARENT);
    DrawTextW(dc, s.c_str(), int(s.size()), &r, flags | DT_NOPREFIX);
}

void Panel::fill(HDC dc, const RECT &r, COLORREF c) const {
    SetDCBrushColor(dc, c);
    FillRect(dc, &r, static_cast<HBRUSH>(GetStockObject(DC_BRUSH)));
}

void Panel::frame(HDC dc, const RECT &r, COLORREF c) const {
    SetDCBrushColor(dc, c);
    FrameRect(dc, &r, static_cast<HBRUSH>(GetStockObject(DC_BRUSH)));
}

SIZE Panel::control_size(const Control &c) const {
    switch (c.kind) {
    case Kind::Accent: {
        SIZE t = text_size(fonts_.bold, c.text);
        return {t.cx + S(36), t.cy + S(14)};
    }
    case Kind::Info: {
        SIZE t = text_size(fonts_.info, c.text);
        return {t.cx + S(16), t.cy + S(4)};
    }
    case Kind::Link:
        return text_size(fonts_.normal, c.text);
    case Kind::Check: {
        SIZE t = text_size(fonts_.normal, c.text);
        return {t.cx + S(24), std::max<LONG>(t.cy, S(16))};
    }
    case Kind::Segment: {
        SIZE t = text_size(fonts_.normal, c.text);
        return {t.cx + S(24), t.cy + S(6)};
    }
    default: {
        SIZE t = text_size(fonts_.normal, c.text);
        return {t.cx + S(20), t.cy + S(8)};
    }
    }
}

HWND Panel::make_edit(bool multiline) {
    DWORD style = WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL;
    if (multiline) style = WS_CHILD | WS_VSCROLL | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL;
    HWND e = CreateWindowExW(0, L"EDIT", L"", style, 0, 0, 10, 10, hwnd_, nullptr, GetModuleHandleW(nullptr), nullptr);
    SendMessageW(e, WM_SETFONT, WPARAM(multiline ? fonts_.mono : fonts_.normal), TRUE);
    if (multiline) {
        SendMessageW(e, EM_SETLIMITTEXT, 0, 0);
        SetWindowTheme(e, L"DarkMode_Explorer", nullptr);  // dark scroll bar
    }
    return e;
}

void Panel::relayout() {
    RECT c;
    GetClientRect(hwnd_, &c);
    layout(c.right);
    redraw();
}

void Panel::draw_control(HDC dc, int i) const {
    const Control &c = controls_[size_t(i)];
    if (!c.visible) return;
    bool hot = hover_ == i && c.enabled;
    RECT r = c.r;
    switch (c.kind) {
    case Kind::Button:
        fill(dc, r, hot ? LINE : CARD);
        frame(dc, r, BUTTON_BORDER);
        text(dc, c.text, r, fonts_.normal, c.enabled ? TEXT : MUTED, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        break;
    case Kind::Accent:
        fill(dc, r, !c.enabled ? LINE : hot ? ACCENT_HOVER : ACCENT);
        text(dc, c.text, r, fonts_.bold, c.enabled ? ON_ACCENT : MUTED, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        break;
    case Kind::Info:
        fill(dc, r, hot ? LINE : CARD);
        text(dc, c.text, r, fonts_.info, c.enabled ? ACCENT : MUTED, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        break;
    case Kind::Link:
        text(dc, c.text, r, fonts_.normal, hot ? TEXT : MUTED, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        break;
    case Kind::Segment:
        fill(dc, r, c.checked ? ACCENT : hot ? LINE : BG);
        frame(dc, r, LINE);
        text(dc, c.text, r, fonts_.normal, c.checked ? ON_ACCENT : c.enabled ? TEXT : MUTED,
             DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        break;
    case Kind::Check: {
        int box = S(14), y = r.top + (r.bottom - r.top - box) / 2;
        RECT b{r.left, y, r.left + box, y + box};
        fill(dc, b, c.enabled ? BG : CARD);
        frame(dc, b, hot ? ACCENT : LINE);
        if (c.checked) {
            HPEN pen = CreatePen(PS_SOLID, std::max(1, S(2)), c.enabled ? ACCENT : MUTED);
            HGDIOBJ old = SelectObject(dc, pen);
            POINT pts[3] = {{b.left + S(3), b.top + S(7)}, {b.left + S(6), b.top + S(10)}, {b.left + S(11), b.top + S(4)}};
            Polyline(dc, pts, 3);
            SelectObject(dc, old);
            DeleteObject(pen);
        }
        RECT t{r.left + box + S(8), r.top, r.right, r.bottom};
        text(dc, c.text, t, fonts_.normal, c.enabled ? TEXT : MUTED, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        break;
    }
    }
}

int Panel::hit(POINT p) const {
    for (size_t i = 0; i < controls_.size(); i++) {
        const Control &c = controls_[i];
        if (c.visible && PtInRect(&c.r, p)) return int(i);
    }
    return -1;
}

void Panel::set_dpi(int dpi) {
    dpi_ = dpi;
    fonts_.make(dpi);
    for (HWND ch = GetWindow(hwnd_, GW_CHILD); ch; ch = GetWindow(ch, GW_HWNDNEXT)) {
        LONG style = GetWindowLongW(ch, GWL_STYLE);
        SendMessageW(ch, WM_SETFONT, WPARAM(style & ES_MULTILINE ? fonts_.mono : fonts_.normal), TRUE);
    }
}

LRESULT Panel::message(UINT, WPARAM, LPARAM, bool &handled) {
    handled = false;
    return 0;
}

LRESULT CALLBACK Panel::proc(HWND h, UINT msg, WPARAM wp, LPARAM lp) {
    Panel *p = reinterpret_cast<Panel *>(GetWindowLongPtrW(h, GWLP_USERDATA));
    if (msg == WM_NCCREATE) {
        p = static_cast<Panel *>(reinterpret_cast<CREATESTRUCTW *>(lp)->lpCreateParams);
        SetWindowLongPtrW(h, GWLP_USERDATA, LONG_PTR(p));
        p->hwnd_ = h;
        p->set_dpi(int(GetDpiForWindow(h)));
    }
    return p ? p->handle(msg, wp, lp) : DefWindowProcW(h, msg, wp, lp);
}

LRESULT Panel::handle(UINT msg, WPARAM wp, LPARAM lp) {
    bool handled = true;
    LRESULT r = message(msg, wp, lp, handled);
    if (handled) return r;
    switch (msg) {
    case WM_SIZE:
        relayout();
        return 0;
    case WM_ERASEBKGND:
        return 1;
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC dc = BeginPaint(hwnd_, &ps);
        RECT c;
        GetClientRect(hwnd_, &c);
        HDC mem = CreateCompatibleDC(dc);
        HBITMAP bmp = CreateCompatibleBitmap(dc, c.right, c.bottom);
        HGDIOBJ old = SelectObject(mem, bmp);
        paint(mem, c);
        for (size_t i = 0; i < controls_.size(); i++) draw_control(mem, int(i));
        BitBlt(dc, 0, 0, c.right, c.bottom, mem, 0, 0, SRCCOPY);
        SelectObject(mem, old);
        DeleteObject(bmp);
        DeleteDC(mem);
        EndPaint(hwnd_, &ps);
        return 0;
    }
    case WM_MOUSEMOVE: {
        int h = hit({GET_X_LPARAM(lp), GET_Y_LPARAM(lp)});
        if (h != hover_) {
            hover_ = h;
            redraw();
            TRACKMOUSEEVENT t{sizeof t, TME_LEAVE, hwnd_, 0};
            TrackMouseEvent(&t);
        }
        return 0;
    }
    case WM_MOUSELEAVE:
        hover_ = -1;
        redraw();
        return 0;
    case WM_SETCURSOR:
        if (LOWORD(lp) == HTCLIENT && hover_ >= 0 && controls_[size_t(hover_)].enabled) {
            SetCursor(LoadCursorW(nullptr, IDC_HAND));
            return TRUE;
        }
        break;
    case WM_LBUTTONDOWN:
        pressed_ = hit({GET_X_LPARAM(lp), GET_Y_LPARAM(lp)});
        SetCapture(hwnd_);
        return 0;
    case WM_LBUTTONUP: {
        ReleaseCapture();
        int h = hit({GET_X_LPARAM(lp), GET_Y_LPARAM(lp)});
        if (h >= 0 && h == pressed_ && controls_[size_t(h)].enabled) {
            Control &c = controls_[size_t(h)];
            if (c.kind == Kind::Check) c.checked = !c.checked;
            auto f = c.click;
            if (f) f();
            redraw();
        }
        pressed_ = -1;
        return 0;
    }
    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORSTATIC: {
        HDC dc = HDC(wp);
        bool off = !IsWindowEnabled(HWND(lp));
        bool multi = GetWindowLongW(HWND(lp), GWL_STYLE) & ES_MULTILINE;
        SetTextColor(dc, off ? MUTED : multi ? MUTED : TEXT);
        SetBkColor(dc, off ? CARD : BG);
        return LRESULT(off ? card_brush_ : field_brush_);
    }
    case WM_DPICHANGED: {
        set_dpi(HIWORD(wp));
        RECT *s = reinterpret_cast<RECT *>(lp);
        SetWindowPos(hwnd_, nullptr, s->left, s->top, s->right - s->left, s->bottom - s->top, SWP_NOZORDER | SWP_NOACTIVATE);
        relayout();
        return 0;
    }
    }
    return DefWindowProcW(hwnd_, msg, wp, lp);
}

}
