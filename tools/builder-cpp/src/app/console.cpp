#include "console.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

void console_attach() {
    // a pipe or file given by the caller is used as it is; otherwise the parent's console
    if (GetFileType(GetStdHandle(STD_OUTPUT_HANDLE)) == FILE_TYPE_UNKNOWN && AttachConsole(ATTACH_PARENT_PROCESS)) {
        SetConsoleOutputCP(CP_UTF8);
    }
}

void console_write(const std::string &s) {
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    if (h == nullptr || h == INVALID_HANDLE_VALUE) {
        h = CreateFileW(L"CONOUT$", GENERIC_WRITE, FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
        if (h == INVALID_HANDLE_VALUE) return;
        SetStdHandle(STD_OUTPUT_HANDLE, h);
    }
    DWORD n;
    WriteFile(h, s.data(), DWORD(s.size()), &n, nullptr);
}
