#include "process.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <mutex>

namespace {

std::wstring widen(const std::string &s) {
    if (s.empty()) return {};
    int n = MultiByteToWideChar(CP_UTF8, 0, s.data(), int(s.size()), nullptr, 0);
    std::wstring w(size_t(n), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.data(), int(s.size()), w.data(), n);
    return w;
}

// One argument quoted the way CommandLineToArgvW and the C runtime read it back.
void quote(std::wstring &cmd, const std::wstring &a) {
    if (!a.empty() && a.find_first_of(L" \t\"") == std::wstring::npos) {
        cmd += a;
        return;
    }
    cmd += L'"';
    size_t slashes = 0;
    for (wchar_t c : a) {
        if (c == L'\\') {
            slashes++;
        } else if (c == L'"') {
            cmd.append(slashes * 2 + 1, L'\\');
            cmd += c;
            slashes = 0;
        } else {
            cmd.append(slashes, L'\\');
            cmd += c;
            slashes = 0;
        }
    }
    cmd.append(slashes * 2, L'\\');
    cmd += L'"';
}

std::wstring environment_with(const std::map<std::string, std::string> &extra) {
    std::map<std::wstring, std::wstring, std::less<>> vars;
    wchar_t *block = GetEnvironmentStringsW();
    for (wchar_t *p = block; *p; p += wcslen(p) + 1) {
        std::wstring kv = p;
        size_t eq = kv.find(L'=', 1);
        if (eq != std::wstring::npos) vars[kv.substr(0, eq)] = kv.substr(eq + 1);
    }
    FreeEnvironmentStringsW(block);
    for (auto &[k, v] : extra) vars[widen(k)] = widen(v);
    std::wstring env;
    for (auto &[k, v] : vars) env += k + L"=" + v + L'\0';
    env += L'\0';
    return env;
}

std::mutex g_create_mutex;      // handles must not leak into a child started at the same moment

}

RunResult run_program(const fs::path &exe, const std::vector<std::string> &args, const fs::path &cwd,
                      const std::map<std::string, std::string> &env, bool stdout_only) {
    std::wstring cmd;
    quote(cmd, exe.wstring());
    for (auto &a : args) {
        cmd += L' ';
        quote(cmd, widen(a));
    }
    SECURITY_ATTRIBUTES sa{sizeof sa, nullptr, TRUE};
    HANDLE rd = nullptr, wr = nullptr;
    PROCESS_INFORMATION pi{};
    std::wstring envblock = env.empty() ? std::wstring() : environment_with(env);
    {
        std::lock_guard<std::mutex> lock(g_create_mutex);
        if (!CreatePipe(&rd, &wr, &sa, 0)) throw Error("cannot create a pipe");
        SetHandleInformation(rd, HANDLE_FLAG_INHERIT, 0);
        HANDLE nul = CreateFileW(L"NUL", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, &sa,
                                 OPEN_EXISTING, 0, nullptr);
        STARTUPINFOEXW si{};
        si.StartupInfo.cb = sizeof si;
        si.StartupInfo.dwFlags = STARTF_USESTDHANDLES;
        si.StartupInfo.hStdInput = nul;
        si.StartupInfo.hStdOutput = wr;
        si.StartupInfo.hStdError = stdout_only ? nul : wr;
        // the child inherits only its pipe and NUL, not the files other levels' threads have open at this moment
        // (an inherited handle keeps such a file open, so its thread cannot delete it)
        HANDLE inherit[2] = {wr, nul};
        SIZE_T attr_size = 0;
        InitializeProcThreadAttributeList(nullptr, 1, 0, &attr_size);
        std::vector<uint8_t> attr_buf(attr_size);
        si.lpAttributeList = reinterpret_cast<LPPROC_THREAD_ATTRIBUTE_LIST>(attr_buf.data());
        InitializeProcThreadAttributeList(si.lpAttributeList, 1, 0, &attr_size);
        UpdateProcThreadAttribute(si.lpAttributeList, 0, PROC_THREAD_ATTRIBUTE_HANDLE_LIST, inherit, sizeof inherit,
                                  nullptr, nullptr);
        BOOL ok = CreateProcessW(nullptr, cmd.data(), nullptr, nullptr, TRUE,
                                 CREATE_NO_WINDOW | EXTENDED_STARTUPINFO_PRESENT | (env.empty() ? 0 : CREATE_UNICODE_ENVIRONMENT),
                                 env.empty() ? nullptr : envblock.data(), cwd.wstring().c_str(), &si.StartupInfo, &pi);
        DeleteProcThreadAttributeList(si.lpAttributeList);
        CloseHandle(wr);
        CloseHandle(nul);
        if (!ok) {
            CloseHandle(rd);
            throw Error("cannot start " + path_str(exe.filename()));
        }
    }
    RunResult r;
    char buf[65536];
    DWORD n;
    while (ReadFile(rd, buf, sizeof buf, &n, nullptr) && n) r.out.append(buf, n);
    CloseHandle(rd);
    WaitForSingleObject(pi.hProcess, INFINITE);
    DWORD code = 0;
    GetExitCodeProcess(pi.hProcess, &code);
    r.exit_code = int(code);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return r;
}

fs::path exe_folder() {
    std::wstring buf(MAX_PATH, wchar_t(0));
    for (;;) {
        DWORD n = GetModuleFileNameW(nullptr, buf.data(), DWORD(buf.size()));
        if (n < buf.size()) return fs::path(buf.substr(0, n)).parent_path();
        buf.resize(buf.size() * 2);
    }
}

fs::path program_dir() { return exe_folder() / "tools"; }

fs::path program_path(const std::string &name) {
    fs::path p = program_dir() / u8path(name + ".exe");
    if (!fs::is_regular_file(p))
        throw Error("the conversion program " + path_str(p) + " is missing; unpack the whole ZIP, with its tools folder.");
    return p;
}

std::string run_tool_program(const std::string &name, const std::vector<std::string> &args, const fs::path &cwd,
                             const std::map<std::string, std::string> &env) {
    RunResult r = run_program(program_path(name), args, cwd, env);
    if (r.exit_code != 0) {
        std::string head;
        for (size_t i = 0; i < args.size() && i < 3; i++) head += " " + args[i];
        std::string tail = r.out.size() > 800 ? r.out.substr(r.out.size() - 800) : r.out;
        throw Error(name + head + ": exit " + std::to_string(r.exit_code) + "\n" + tail);
    }
    return r.out;
}
