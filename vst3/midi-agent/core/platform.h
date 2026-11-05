// Thin file-system and process helpers. Windows is the target DAW platform;
// POSIX is kept working so the core builds and tests on Linux CI.
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace midiagent {
namespace platform {

bool readFile (const std::string& path, std::string& out, std::string& error);
bool writeFile (const std::string& path, const std::string& data, std::string& error);
bool writeFile (const std::string& path, const std::vector<uint8_t>& data, std::string& error);
bool fileExists (const std::string& path);
bool dirExists (const std::string& path);
bool makeDirs (const std::string& path);
/// Files directly inside `dir` whose name ends with `suffix`, sorted by name.
std::vector<std::string> listFiles (const std::string& dir, const std::string& suffix);
std::string joinPath (const std::string& a, const std::string& b);
std::string parentPath (const std::string& path);
std::string fileName (const std::string& path);
std::string stem (const std::string& path);
/// A writable folder for exported .mid / .wav drag files.
std::string tempDir ();
/// Milliseconds since an arbitrary epoch, for unique names.
int64_t nowMillis ();
/// "untitled song!" -> "untitled_song_" like generate_midi._safe.
std::string safeName (const std::string& title);

struct ProcessResult
{
    bool started {false};
    int exitCode {-1};
    std::string output; // stdout and stderr, merged
    std::string error;  // why the process could not start
};

/// Run a program and wait for it. Blocking: call this from a worker thread.
ProcessResult runProcess (const std::vector<std::string>& argv, const std::string& workingDir);
/// Quote one argument for a Windows command line (CommandLineToArgvW rules).
std::string quoteWindowsArg (const std::string& arg);

} // namespace platform
} // namespace midiagent
