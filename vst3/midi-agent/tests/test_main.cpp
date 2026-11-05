#include "tests/test_framework.h"

#include <cstdio>

namespace midiagent_test {

std::vector<TestCase>& registry ()
{
    static std::vector<TestCase> r;
    return r;
}

int& failures ()
{
    static int f = 0;
    return f;
}

void reportFailure (const char* file, int line, const std::string& message)
{
    ++failures ();
    std::fprintf (stderr, "  %s:%d: %s\n", file, line, message.c_str ());
}

} // namespace midiagent_test

int main ()
{
    int ran = 0, failed = 0;
    for (const auto& t : midiagent_test::registry ())
    {
        int before = midiagent_test::failures ();
        std::fprintf (stderr, "[ RUN  ] %s\n", t.name);
        try
        {
            t.body ();
        }
        catch (const std::exception& e)
        {
            midiagent_test::reportFailure ("<exception>", 0, e.what ());
        }
        ++ran;
        if (midiagent_test::failures () != before)
        {
            ++failed;
            std::fprintf (stderr, "[ FAIL ] %s\n", t.name);
        }
        else
            std::fprintf (stderr, "[  OK  ] %s\n", t.name);
    }
    std::fprintf (stderr, "%d tests, %d failed\n", ran, failed);
    return failed == 0 ? 0 : 1;
}
