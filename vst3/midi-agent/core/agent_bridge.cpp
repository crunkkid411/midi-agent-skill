#include "core/agent_bridge.h"
#include "core/platform.h"

#include <cstdio>

namespace midiagent {

std::string joinList (const std::vector<std::string>& items, const std::string& sep)
{
    std::string out;
    for (size_t i = 0; i < items.size (); ++i)
    {
        if (i)
            out += sep;
        out += items[i];
    }
    return out;
}

static std::string composeScript (const AgentConfig& cfg)
{
    return platform::joinPath (platform::joinPath (cfg.skillRoot, "skills"), "compose_from_profile.py");
}

namespace {
std::string num (double v)
{
    char buf[32];
    std::snprintf (buf, sizeof (buf), "%g", v);
    return buf;
}

void pushWeightsAndVariation (std::vector<std::string>& a, const std::vector<std::string>& genres,
                              const std::vector<double>& weights, double variation)
{
    if (!weights.empty () && weights.size () == genres.size ())
    {
        std::vector<std::string> w;
        for (double v : weights)
            w.push_back (num (v));
        a.push_back ("--weights");
        a.push_back (joinList (w));
    }
    if (variation > 0.0)
    {
        a.push_back ("--variation");
        a.push_back (num (variation > 1.0 ? 1.0 : variation));
    }
}
} // namespace

std::vector<std::string> buildComposeArgs (const AgentConfig& cfg, const ComposeRequest& req, const std::string& outPath)
{
    std::vector<std::string> a {cfg.pythonExe, composeScript (cfg)};
    if (!req.genres.empty ())
    {
        a.push_back ("--genres");
        a.push_back (joinList (req.genres));
    }
    pushWeightsAndVariation (a, req.genres, req.weights, req.variation);
    if (req.useForm)
        a.push_back ("--form");
    else
    {
        a.push_back ("--bars");
        a.push_back (std::to_string (req.bars > 0 ? req.bars : 8));
    }
    if (req.bpm > 0)
    {
        a.push_back ("--bpm");
        a.push_back (std::to_string (req.bpm));
    }
    if (!req.key.empty ())
    {
        a.push_back ("--key");
        a.push_back (req.key);
    }
    a.push_back ("--title");
    a.push_back (req.title.empty () ? "midi-agent" : req.title);
    if (req.seed >= 0)
    {
        a.push_back ("--seed");
        a.push_back (std::to_string (req.seed));
    }
    if (!req.vstMap.empty ())
    {
        a.push_back ("--vst");
        a.push_back (req.vstMap);
    }
    a.push_back ("--out");
    a.push_back (outPath);
    a.push_back ("--no-midi"); // the plug-in writes its own .mid files
    return a;
}

std::vector<std::string> buildRegenerateArgs (const AgentConfig& cfg, const RegenerateRequest& req, const std::string& outPath)
{
    std::vector<std::string> a {cfg.pythonExe, composeScript (cfg)};
    if (!req.genres.empty ())
    {
        a.push_back ("--genres");
        a.push_back (joinList (req.genres));
    }
    pushWeightsAndVariation (a, req.genres, req.weights, req.variation);
    a.push_back ("--resume");
    a.push_back (req.resumePath);
    a.push_back ("--regenerate");
    a.push_back (req.section);
    if (!req.layers.empty ())
    {
        a.push_back ("--layers");
        a.push_back (joinList (req.layers));
    }
    if (req.seed >= 0)
    {
        a.push_back ("--seed");
        a.push_back (std::to_string (req.seed));
    }
    a.push_back ("--out");
    a.push_back (outPath);
    a.push_back ("--no-midi");
    return a;
}

std::vector<std::string> buildConvertToWavArgs (const AgentConfig& cfg, const std::string& midiPath)
{
    return {cfg.pythonExe, platform::joinPath (platform::joinPath (cfg.skillRoot, "skills"), "convert_to_wav.py"), midiPath};
}

static AgentResult runAndLoad (const AgentConfig& cfg, const std::vector<std::string>& argv, const std::string& outPath)
{
    AgentResult r;
    if (!cfg.valid ())
    {
        r.error = "skill folder is not set";
        return r;
    }
    if (!platform::fileExists (composeScript (cfg)))
    {
        r.error = "compose_from_profile.py not found under " + cfg.skillRoot;
        return r;
    }
    platform::makeDirs (platform::parentPath (outPath));
    platform::ProcessResult p = platform::runProcess (argv, cfg.skillRoot);
    r.log = p.output;
    if (!p.started)
    {
        r.error = p.error.empty () ? "could not start python" : p.error;
        return r;
    }
    if (p.exitCode != 0)
    {
        r.error = "python exited with code " + std::to_string (p.exitCode);
        return r;
    }
    if (!platform::fileExists (outPath))
    {
        r.error = "python finished but wrote no composition at " + outPath;
        return r;
    }
    r.ok = true;
    r.compositionPath = outPath;
    return r;
}

AgentResult runCompose (const AgentConfig& cfg, const ComposeRequest& req, const std::string& outPath)
{
    return runAndLoad (cfg, buildComposeArgs (cfg, req, outPath), outPath);
}

AgentResult runRegenerate (const AgentConfig& cfg, const RegenerateRequest& req, const std::string& outPath)
{
    return runAndLoad (cfg, buildRegenerateArgs (cfg, req, outPath), outPath);
}

static std::vector<std::string> listSlugs (const std::string& dir, const std::string& skip)
{
    std::vector<std::string> out;
    for (const std::string& f : platform::listFiles (dir, ".json"))
    {
        if (f == skip || (!f.empty () && f[0] == '_'))
            continue;
        out.push_back (platform::stem (f));
    }
    return out;
}

std::vector<std::string> listGenres (const AgentConfig& cfg)
{
    return listSlugs (platform::joinPath (cfg.skillRoot, "genres"), "_schema.json");
}

std::vector<std::string> listInstrumentMaps (const AgentConfig& cfg)
{
    return listSlugs (platform::joinPath (cfg.skillRoot, "instruments"), "_template.json");
}

std::string findSkillRoot (const std::string& start)
{
    std::string dir = start;
    for (int depth = 0; depth < 8 && !dir.empty (); ++depth)
    {
        if (platform::fileExists (platform::joinPath (dir, "SKILL.md")) &&
            platform::fileExists (platform::joinPath (platform::joinPath (dir, "skills"), "compose_from_profile.py")))
            return dir;
        std::string parent = platform::parentPath (dir);
        if (parent == dir)
            break;
        dir = parent;
    }
    return "";
}

} // namespace midiagent
