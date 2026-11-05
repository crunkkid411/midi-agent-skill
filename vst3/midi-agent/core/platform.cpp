#include "core/platform.h"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <fstream>
#include <sstream>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <direct.h>
#include <io.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

namespace midiagent {
namespace platform {

#ifdef _WIN32
static std::wstring toWide (const std::string& utf8)
{
    if (utf8.empty ())
        return {};
    int n = MultiByteToWideChar (CP_UTF8, 0, utf8.c_str (), -1, nullptr, 0);
    std::wstring w (static_cast<size_t> (n > 0 ? n - 1 : 0), L'\0');
    if (n > 1)
        MultiByteToWideChar (CP_UTF8, 0, utf8.c_str (), -1, &w[0], n);
    return w;
}

static std::string fromWide (const std::wstring& w)
{
    if (w.empty ())
        return {};
    int n = WideCharToMultiByte (CP_UTF8, 0, w.c_str (), -1, nullptr, 0, nullptr, nullptr);
    std::string s (static_cast<size_t> (n > 0 ? n - 1 : 0), '\0');
    if (n > 1)
        WideCharToMultiByte (CP_UTF8, 0, w.c_str (), -1, &s[0], n, nullptr, nullptr);
    return s;
}
#endif

bool readFile (const std::string& path, std::string& out, std::string& error)
{
#ifdef _WIN32
    std::ifstream f (toWide (path), std::ios::binary);
#else
    std::ifstream f (path, std::ios::binary);
#endif
    if (!f)
    {
        error = "cannot open " + path;
        return false;
    }
    std::ostringstream ss;
    ss << f.rdbuf ();
    out = ss.str ();
    return true;
}

bool writeFile (const std::string& path, const std::string& data, std::string& error)
{
#ifdef _WIN32
    std::ofstream f (toWide (path), std::ios::binary | std::ios::trunc);
#else
    std::ofstream f (path, std::ios::binary | std::ios::trunc);
#endif
    if (!f)
    {
        error = "cannot write " + path;
        return false;
    }
    f.write (data.data (), static_cast<std::streamsize> (data.size ()));
    if (!f)
    {
        error = "write failed for " + path;
        return false;
    }
    return true;
}

bool writeFile (const std::string& path, const std::vector<uint8_t>& data, std::string& error)
{
    return writeFile (path, std::string (data.begin (), data.end ()), error);
}

bool fileExists (const std::string& path)
{
#ifdef _WIN32
    DWORD a = GetFileAttributesW (toWide (path).c_str ());
    return a != INVALID_FILE_ATTRIBUTES && !(a & FILE_ATTRIBUTE_DIRECTORY);
#else
    struct stat st;
    return ::stat (path.c_str (), &st) == 0 && S_ISREG (st.st_mode);
#endif
}

bool dirExists (const std::string& path)
{
#ifdef _WIN32
    DWORD a = GetFileAttributesW (toWide (path).c_str ());
    return a != INVALID_FILE_ATTRIBUTES && (a & FILE_ATTRIBUTE_DIRECTORY);
#else
    struct stat st;
    return ::stat (path.c_str (), &st) == 0 && S_ISDIR (st.st_mode);
#endif
}

bool makeDirs (const std::string& path)
{
    if (path.empty ())
        return false;
    if (dirExists (path))
        return true;
    std::string parent = parentPath (path);
    if (!parent.empty () && parent != path && !dirExists (parent))
        if (!makeDirs (parent))
            return false;
#ifdef _WIN32
    return CreateDirectoryW (toWide (path).c_str (), nullptr) != 0 || dirExists (path);
#else
    return ::mkdir (path.c_str (), 0755) == 0 || dirExists (path);
#endif
}

std::vector<std::string> listFiles (const std::string& dir, const std::string& suffix)
{
    std::vector<std::string> out;
#ifdef _WIN32
    WIN32_FIND_DATAW fd;
    HANDLE h = FindFirstFileW (toWide (joinPath (dir, "*")).c_str (), &fd);
    if (h == INVALID_HANDLE_VALUE)
        return out;
    do
    {
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            continue;
        std::string name = fromWide (fd.cFileName);
        if (name.size () >= suffix.size () && name.compare (name.size () - suffix.size (), suffix.size (), suffix) == 0)
            out.push_back (name);
    } while (FindNextFileW (h, &fd));
    FindClose (h);
#else
    DIR* d = ::opendir (dir.c_str ());
    if (!d)
        return out;
    while (dirent* e = ::readdir (d))
    {
        std::string name = e->d_name;
        if (name == "." || name == "..")
            continue;
        if (!fileExists (joinPath (dir, name)))
            continue;
        if (name.size () >= suffix.size () && name.compare (name.size () - suffix.size (), suffix.size (), suffix) == 0)
            out.push_back (name);
    }
    ::closedir (d);
#endif
    std::sort (out.begin (), out.end ());
    return out;
}

std::string joinPath (const std::string& a, const std::string& b)
{
    if (a.empty ())
        return b;
    if (b.empty ())
        return a;
    char last = a.back ();
    if (last == '/' || last == '\\')
        return a + b;
#ifdef _WIN32
    return a + "\\" + b;
#else
    return a + "/" + b;
#endif
}

std::string parentPath (const std::string& path)
{
    size_t p = path.find_last_of ("/\\");
    if (p == std::string::npos)
        return "";
    if (p == 0)
        return path.substr (0, 1);
    return path.substr (0, p);
}

std::string fileName (const std::string& path)
{
    size_t p = path.find_last_of ("/\\");
    return p == std::string::npos ? path : path.substr (p + 1);
}

std::string stem (const std::string& path)
{
    std::string n = fileName (path);
    size_t d = n.find_last_of ('.');
    return d == std::string::npos ? n : n.substr (0, d);
}

std::string tempDir ()
{
#ifdef _WIN32
    wchar_t buf[MAX_PATH + 1];
    DWORD n = GetTempPathW (MAX_PATH, buf);
    std::string base = n ? fromWide (std::wstring (buf, n)) : std::string ("C:\\Temp\\");
#else
    const char* t = std::getenv ("TMPDIR");
    std::string base = t ? t : "/tmp";
#endif
    std::string dir = joinPath (base, "midi-agent");
    makeDirs (dir);
    return dir;
}

int64_t nowMillis ()
{
    using namespace std::chrono;
    return duration_cast<milliseconds> (steady_clock::now ().time_since_epoch ()).count ();
}

std::string safeName (const std::string& title)
{
    std::string out;
    for (char c : title)
    {
        bool ok = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_' || c == '-';
        out += ok ? c : '_';
    }
    if (out.empty ())
        out = "untitled";
    return out;
}

std::string quoteWindowsArg (const std::string& arg)
{
    if (!arg.empty () && arg.find_first_of (" \t\n\"") == std::string::npos)
        return arg;
    std::string out = "\"";
    size_t backslashes = 0;
    for (char c : arg)
    {
        if (c == '\\')
        {
            ++backslashes;
            continue;
        }
        if (c == '"')
        {
            out.append (backslashes * 2 + 1, '\\');
            out += '"';
            backslashes = 0;
            continue;
        }
        out.append (backslashes, '\\');
        backslashes = 0;
        out += c;
    }
    out.append (backslashes * 2, '\\');
    out += '"';
    return out;
}

ProcessResult runProcess (const std::vector<std::string>& argv, const std::string& workingDir)
{
    ProcessResult r;
    if (argv.empty ())
    {
        r.error = "empty command";
        return r;
    }
#ifdef _WIN32
    std::string cmdline;
    for (size_t i = 0; i < argv.size (); ++i)
    {
        if (i)
            cmdline += ' ';
        cmdline += quoteWindowsArg (argv[i]);
    }
    SECURITY_ATTRIBUTES sa {};
    sa.nLength = sizeof (sa);
    sa.bInheritHandle = TRUE;
    HANDLE readPipe = nullptr, writePipe = nullptr;
    if (!CreatePipe (&readPipe, &writePipe, &sa, 0))
    {
        r.error = "CreatePipe failed";
        return r;
    }
    SetHandleInformation (readPipe, HANDLE_FLAG_INHERIT, 0);

    STARTUPINFOW si {};
    si.cb = sizeof (si);
    si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.hStdOutput = writePipe;
    si.hStdError = writePipe;
    si.hStdInput = GetStdHandle (STD_INPUT_HANDLE);
    si.wShowWindow = SW_HIDE; // no console window flashing over Cubase
    PROCESS_INFORMATION pi {};
    std::wstring wcmd = toWide (cmdline);
    std::wstring wcwd = toWide (workingDir);
    BOOL ok = CreateProcessW (nullptr, &wcmd[0], nullptr, nullptr, TRUE, CREATE_NO_WINDOW, nullptr,
                              workingDir.empty () ? nullptr : wcwd.c_str (), &si, &pi);
    CloseHandle (writePipe);
    if (!ok)
    {
        CloseHandle (readPipe);
        r.error = "CreateProcess failed (" + std::to_string (GetLastError ()) + "): " + cmdline;
        return r;
    }
    r.started = true;
    char buf[4096];
    DWORD got = 0;
    while (ReadFile (readPipe, buf, sizeof (buf), &got, nullptr) && got > 0)
        r.output.append (buf, got);
    CloseHandle (readPipe);
    WaitForSingleObject (pi.hProcess, INFINITE);
    DWORD code = 0;
    GetExitCodeProcess (pi.hProcess, &code);
    r.exitCode = static_cast<int> (code);
    CloseHandle (pi.hProcess);
    CloseHandle (pi.hThread);
#else
    int fds[2];
    if (::pipe (fds) != 0)
    {
        r.error = "pipe failed";
        return r;
    }
    pid_t pid = ::fork ();
    if (pid < 0)
    {
        r.error = "fork failed";
        return r;
    }
    if (pid == 0)
    {
        ::dup2 (fds[1], 1);
        ::dup2 (fds[1], 2);
        ::close (fds[0]);
        ::close (fds[1]);
        if (!workingDir.empty ())
            if (::chdir (workingDir.c_str ()) != 0)
                _exit (126);
        std::vector<char*> args;
        for (const auto& a : argv)
            args.push_back (const_cast<char*> (a.c_str ()));
        args.push_back (nullptr);
        ::execvp (args[0], args.data ());
        _exit (127);
    }
    ::close (fds[1]);
    r.started = true;
    char buf[4096];
    ssize_t got;
    while ((got = ::read (fds[0], buf, sizeof (buf))) > 0)
        r.output.append (buf, static_cast<size_t> (got));
    ::close (fds[0]);
    int status = 0;
    ::waitpid (pid, &status, 0);
    r.exitCode = WIFEXITED (status) ? WEXITSTATUS (status) : -1;
    if (r.exitCode == 127)
        r.error = "program not found: " + argv[0];
#endif
    return r;
}

} // namespace platform
} // namespace midiagent
