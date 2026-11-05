#include "core/transport_remote.h"

#include <cmath>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
#endif

namespace midiagent {

std::vector<unsigned char> mmcLocateMessage (double seconds)
{
    if (seconds < 0)
        seconds = 0;
    const int fps = 30;
    long long totalFrames = static_cast<long long> (std::floor (seconds * fps));
    double frac = seconds * fps - static_cast<double> (totalFrames);
    int frames = static_cast<int> (totalFrames % fps);
    long long totalSec = totalFrames / fps;
    int sec = static_cast<int> (totalSec % 60);
    int min = static_cast<int> ((totalSec / 60) % 60);
    int hr = static_cast<int> ((totalSec / 3600) % 24);
    int sub = static_cast<int> (std::floor (frac * 100)); // subframes 0..99
    // F0 7F <dev 7F=all> 06 (MMC command) 44 (LOCATE) 06 (len) 01 (TARGET) hr mn sc fr ff F7
    // hr carries the frame rate in bits 5-6: 3 = 30 fps non drop
    return {0xF0, 0x7F, 0x7F, 0x06, 0x44, 0x06, 0x01, static_cast<unsigned char> ((3 << 5) | hr),
            static_cast<unsigned char> (min), static_cast<unsigned char> (sec), static_cast<unsigned char> (frames),
            static_cast<unsigned char> (sub), 0xF7};
}

TransportRemote::~TransportRemote ()
{
    close ();
}

bool TransportRemote::isOpen () const
{
    std::lock_guard<std::mutex> lock (mutex_);
    return handle_ != nullptr;
}

std::string TransportRemote::portName () const
{
    std::lock_guard<std::mutex> lock (mutex_);
    return name_;
}

#ifdef _WIN32
bool TransportRemote::open (const std::string& nameContains)
{
    std::lock_guard<std::mutex> lock (mutex_);
    if (handle_)
        return true;
    UINT n = midiOutGetNumDevs ();
    for (UINT i = 0; i < n; ++i)
    {
        MIDIOUTCAPSA caps {};
        if (midiOutGetDevCapsA (i, &caps, sizeof (caps)) != MMSYSERR_NOERROR)
            continue;
        std::string name (caps.szPname);
        if (name.find (nameContains) == std::string::npos)
            continue;
        HMIDIOUT h = nullptr;
        if (midiOutOpen (&h, i, 0, 0, CALLBACK_NULL) == MMSYSERR_NOERROR)
        {
            handle_ = h;
            name_ = name;
            return true;
        }
    }
    return false;
}

bool TransportRemote::locateSeconds (double seconds)
{
    std::vector<unsigned char> msg = mmcLocateMessage (seconds);
    std::lock_guard<std::mutex> lock (mutex_);
    if (!handle_)
        return false;
    HMIDIOUT h = static_cast<HMIDIOUT> (handle_);
    MIDIHDR hdr {};
    hdr.lpData = reinterpret_cast<LPSTR> (msg.data ());
    hdr.dwBufferLength = static_cast<DWORD> (msg.size ());
    hdr.dwBytesRecorded = hdr.dwBufferLength;
    if (midiOutPrepareHeader (h, &hdr, sizeof (hdr)) != MMSYSERR_NOERROR)
        return false;
    bool ok = midiOutLongMsg (h, &hdr, sizeof (hdr)) == MMSYSERR_NOERROR;
    for (int i = 0; i < 200 && !(hdr.dwFlags & MHDR_DONE); ++i)
        Sleep (1);
    midiOutUnprepareHeader (h, &hdr, sizeof (hdr));
    return ok;
}

void TransportRemote::close ()
{
    std::lock_guard<std::mutex> lock (mutex_);
    if (handle_)
    {
        midiOutReset (static_cast<HMIDIOUT> (handle_));
        midiOutClose (static_cast<HMIDIOUT> (handle_));
        handle_ = nullptr;
    }
}
#else
bool TransportRemote::open (const std::string&) { return false; }
bool TransportRemote::locateSeconds (double) { return false; }
void TransportRemote::close () {}
#endif

} // namespace midiagent
