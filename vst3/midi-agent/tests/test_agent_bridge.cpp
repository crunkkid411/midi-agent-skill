#include "core/agent_bridge.h"
#include "core/platform.h"
#include "tests/test_framework.h"

using namespace midiagent;

TEST_CASE (compose_args_match_the_cli)
{
    AgentConfig cfg;
    cfg.pythonExe = "python";
    cfg.skillRoot = "/skill";
    ComposeRequest r;
    r.genres = {"scenecore", "lo-fi-hip-hop"};
    r.weights = {0.6, 0.4};
    r.bars = 16;
    r.title = "song";
    r.seed = 7;
    r.bpm = 150;
    r.key = "A minor";
    auto a = buildComposeArgs (cfg, r, "/out/song.composition.json");
    std::string joined = joinList (a, " ");
    CHECK_EQ (a[0], std::string ("python"));
    CHECK (a[1].find ("compose_from_profile.py") != std::string::npos);
    CHECK (joined.find ("--genres scenecore,lo-fi-hip-hop") != std::string::npos);
    CHECK (joined.find ("--weights 0.6,0.4") != std::string::npos);
    CHECK (joined.find ("--bars 16") != std::string::npos);
    CHECK (joined.find ("--seed 7") != std::string::npos);
    CHECK (joined.find ("--bpm 150") != std::string::npos);
    CHECK (joined.find ("--out /out/song.composition.json") != std::string::npos);
    CHECK (joined.find ("--no-midi") != std::string::npos);
    r.useForm = true;
    auto b = buildComposeArgs (cfg, r, "/out/x.json");
    std::string jb = joinList (b, " ");
    CHECK (jb.find ("--form") != std::string::npos);
    CHECK (jb.find ("--bars") == std::string::npos);
}

TEST_CASE (regenerate_args_match_the_cli)
{
    AgentConfig cfg;
    cfg.skillRoot = "/skill";
    RegenerateRequest r;
    r.resumePath = "/plans/song.composition.json";
    r.section = "verse2.B";
    r.layers = {"drums"};
    r.genres = {"scenecore"};
    r.seed = 17;
    r.variation = 0.5;
    std::string j = joinList (buildRegenerateArgs (cfg, r, "/plans/song.composition.json"), " ");
    CHECK (j.find ("--variation 0.5") != std::string::npos);
    CHECK (j.find ("--resume /plans/song.composition.json") != std::string::npos);
    CHECK (j.find ("--regenerate verse2.B") != std::string::npos);
    CHECK (j.find ("--layers drums") != std::string::npos);
    CHECK (j.find ("--genres scenecore") != std::string::npos);
    CHECK (j.find ("--seed 17") != std::string::npos);
}

TEST_CASE (windows_quoting)
{
    CHECK_EQ (platform::quoteWindowsArg ("plain"), std::string ("plain"));
    CHECK_EQ (platform::quoteWindowsArg ("has space"), std::string ("\"has space\""));
    CHECK_EQ (platform::quoteWindowsArg ("a\"b"), std::string ("\"a\\\"b\""));
    CHECK_EQ (platform::quoteWindowsArg ("C:\\dir\\"), std::string ("C:\\dir\\"));
    CHECK_EQ (platform::quoteWindowsArg ("C:\\my dir\\"), std::string ("\"C:\\my dir\\\\\""));
}

TEST_CASE (skill_root_is_found_from_the_fixture_folder)
{
    // fixtures/ is inside vst3/midi-agent/ inside the skill repository
    std::string root = findSkillRoot (MIDIAGENT_FIXTURE_DIR);
    CHECK (!root.empty ());
    CHECK (platform::fileExists (platform::joinPath (root, "SKILL.md")));
    AgentConfig cfg;
    cfg.skillRoot = root;
    auto genres = listGenres (cfg);
    CHECK (genres.size () >= 5);
    bool hasScenecore = false;
    for (const auto& g : genres)
        if (g == "scenecore")
            hasScenecore = true;
    CHECK (hasScenecore);
    for (const auto& g : genres)
        CHECK (g != "_schema");
    CHECK (listInstrumentMaps (cfg).size () >= 5);
    CHECK_EQ (findSkillRoot ("/"), std::string (""));
}

TEST_CASE (missing_python_is_an_error_with_a_reason)
{
    AgentConfig cfg;
    cfg.pythonExe = "definitely-not-a-python-binary";
    cfg.skillRoot = findSkillRoot (MIDIAGENT_FIXTURE_DIR);
    ComposeRequest r;
    r.genres = {"scenecore"};
    std::string out = platform::joinPath (platform::tempDir (), "never-written.composition.json");
    AgentResult res = runCompose (cfg, r, out);
    CHECK (!res.ok);
    CHECK (!res.error.empty ());
}
