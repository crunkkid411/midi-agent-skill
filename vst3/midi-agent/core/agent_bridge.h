// The bridge to midi-agent-skill's Python CLI. The plug-in does not port the
// composers; it runs skills/compose_from_profile.py and loads the composition
// JSON it writes. This keeps one source of truth for the music logic.
#pragma once
#include "core/composition.h"

#include <string>
#include <vector>

namespace midiagent {

struct AgentConfig
{
    std::string pythonExe {"python"}; // "python" is the Windows launcher name; "python3" elsewhere
    std::string skillRoot;            // folder holding SKILL.md, skills/, genres/
    bool valid () const { return !skillRoot.empty (); }
};

struct ComposeRequest
{
    std::vector<std::string> genres; // slugs from genres/
    std::vector<double> weights;     // optional, same length as genres
    int bars {8};
    bool useForm {false};
    int bpm {0};          // 0 = profile default
    std::string key;      // empty = profile default
    std::string title {"midi-agent"};
    int seed {-1};        // -1 = random
    std::string vstMap;   // "drums=superior-drummer-3,bass=serum-2"
};

struct RegenerateRequest
{
    std::string resumePath;  // the composition JSON to edit
    std::string section;     // "verse2.B"
    std::vector<std::string> layers; // roles to regenerate, empty = all
    std::vector<std::string> genres;
    int seed {-1};
};

struct AgentResult
{
    bool ok {false};
    std::string compositionPath;
    std::string log;   // merged stdout/stderr of the CLI
    std::string error; // short reason when !ok
};

/// Command lines (argv[0] is the Python executable). Exposed for tests and for
/// showing the exact command in the UI log.
std::vector<std::string> buildComposeArgs (const AgentConfig& cfg, const ComposeRequest& req, const std::string& outPath);
std::vector<std::string> buildRegenerateArgs (const AgentConfig& cfg, const RegenerateRequest& req, const std::string& outPath);
std::vector<std::string> buildConvertToWavArgs (const AgentConfig& cfg, const std::string& midiPath);

/// Blocking calls; run them on a worker thread.
AgentResult runCompose (const AgentConfig& cfg, const ComposeRequest& req, const std::string& outPath);
AgentResult runRegenerate (const AgentConfig& cfg, const RegenerateRequest& req, const std::string& outPath);

/// Genre slugs available in <skillRoot>/genres (without _schema.json).
std::vector<std::string> listGenres (const AgentConfig& cfg);
/// Instrument map slugs in <skillRoot>/instruments (without _template.json).
std::vector<std::string> listInstrumentMaps (const AgentConfig& cfg);
/// Walk up from `start` looking for a folder that holds SKILL.md and skills/compose_from_profile.py.
std::string findSkillRoot (const std::string& start);

std::string joinList (const std::vector<std::string>& items, const std::string& sep = ",");

} // namespace midiagent
