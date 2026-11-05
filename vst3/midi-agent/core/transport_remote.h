// Moves the host's playhead. VST3 has no call for that, so the plug-in sends a
// MIDI Machine Control "Locate" SysEx to a MIDI port (a loopMIDI port named
// "MIDI Agent Remote") and Cubase follows it once Studio Setup > Remote
// Devices > MIDI Machine Control Slave listens on that port.
//
// Thread safe: open/locate may be called from any thread (one lock).
#pragma once

#include <mutex>
#include <string>
#include <vector>

namespace midiagent {

/// The MMC Locate message for a position in seconds (30 fps, non drop).
std::vector<unsigned char> mmcLocateMessage (double seconds);

class TransportRemote
{
public:
    static constexpr const char* kDefaultPort = "MIDI Agent Remote";

    ~TransportRemote ();
    /// Open the first MIDI output whose name contains `nameContains`. Safe to
    /// call again; re-opens only if not open.
    bool open (const std::string& nameContains = kDefaultPort);
    bool isOpen () const;
    std::string portName () const;
    bool locateSeconds (double seconds);
    void close ();

private:
    mutable std::mutex mutex_;
    void* handle_ {nullptr};
    std::string name_;
};

} // namespace midiagent
