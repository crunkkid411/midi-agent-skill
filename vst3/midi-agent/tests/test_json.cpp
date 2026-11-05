#include "core/json.h"
#include "tests/test_framework.h"

using namespace midiagent;

TEST_CASE (json_parses_nested_values)
{
    Json v;
    std::string err;
    CHECK (Json::parse (R"({"a": [1, 2.5, "x", true, null], "b": {"c": "d\u00e9\n"}})", v, err));
    CHECK (err.empty ());
    CHECK (v.isObject ());
    CHECK_EQ (v.get ("a").size (), size_t (5));
    CHECK_EQ (v.get ("a").at (0).asInt (), 1);
    CHECK_NEAR (v.get ("a").at (1).asNumber (), 2.5, 1e-12);
    CHECK_EQ (v.get ("a").at (2).asString (), std::string ("x"));
    CHECK (v.get ("a").at (3).asBool ());
    CHECK (v.get ("a").at (4).isNull ());
    CHECK_EQ (v.get ("b").get ("c").asString (), std::string ("d\xC3\xA9\n"));
}

TEST_CASE (json_reports_offset_on_error)
{
    Json v;
    std::string err;
    CHECK (!Json::parse ("{\"a\": [1, 2}", v, err));
    CHECK (err.find ("byte") != std::string::npos);
    CHECK (!Json::parse ("", v, err));
    CHECK (!Json::parse ("{} x", v, err));
}

TEST_CASE (json_round_trips_and_keeps_key_order)
{
    Json v;
    std::string err;
    CHECK (Json::parse (R"({"z": 1, "a": {"q": [], "p": {}}, "m": "s\"q"})", v, err));
    std::string one = v.dump (0);
    CHECK_EQ (one, std::string(R"({"z":1,"a":{"q":[],"p":{}},"m":"s\"q"})"));
    std::string pretty = v.dump (2);
    Json again;
    CHECK (Json::parse (pretty, again, err));
    CHECK_EQ (again.dump (0), one);
}

TEST_CASE (json_numbers_print_like_python)
{
    CHECK_EQ (Json (3.0).dump (), std::string ("3"));
    CHECK_EQ (Json (0.5).dump (), std::string ("0.5"));
    CHECK_EQ (Json (-2).dump (), std::string ("-2"));
    CHECK_EQ (Json (0.549).dump (), std::string ("0.549"));
}

TEST_CASE (json_set_and_erase)
{
    Json o = Json::object ();
    o.set ("a", 1);
    o.set ("b", "x");
    o.set ("a", 2);
    CHECK_EQ (o.size (), size_t (2));
    CHECK_EQ (o.get ("a").asInt (), 2);
    o.erase ("a");
    CHECK (!o.has ("a"));
    CHECK (o.has ("b"));
}
