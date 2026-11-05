// A minimal test registry. No framework dependency: the repository policy is
// standard library only.
#pragma once
#include <cmath>
#include <functional>
#include <sstream>
#include <string>
#include <vector>

namespace midiagent_test {

struct TestCase
{
    const char* name;
    std::function<void ()> body;
};

std::vector<TestCase>& registry ();
int& failures ();

struct Registrar
{
    Registrar (const char* name, std::function<void ()> body) { registry ().push_back ({name, std::move (body)}); }
};

void reportFailure (const char* file, int line, const std::string& message);

} // namespace midiagent_test

#define TEST_CASE(name)                                                          \
    static void name ();                                                         \
    static midiagent_test::Registrar name##_registrar (#name, name);             \
    static void name ()

#define CHECK(cond)                                                                  \
    do                                                                               \
    {                                                                                \
        if (!(cond))                                                                 \
            midiagent_test::reportFailure (__FILE__, __LINE__, "CHECK failed: " #cond); \
    } while (0)

#define CHECK_EQ(a, b)                                                                              \
    do                                                                                              \
    {                                                                                               \
        auto va_ = (a);                                                                             \
        auto vb_ = (b);                                                                             \
        if (!(va_ == vb_))                                                                          \
        {                                                                                           \
            std::ostringstream ss_;                                                                 \
            ss_ << "CHECK_EQ failed: " #a " == " #b " (" << va_ << " vs " << vb_ << ")";            \
            midiagent_test::reportFailure (__FILE__, __LINE__, ss_.str ());                         \
        }                                                                                           \
    } while (0)

#define CHECK_NEAR(a, b, eps)                                                                       \
    do                                                                                              \
    {                                                                                               \
        double va_ = static_cast<double> (a);                                                       \
        double vb_ = static_cast<double> (b);                                                       \
        if (std::fabs (va_ - vb_) > (eps))                                                          \
        {                                                                                           \
            std::ostringstream ss_;                                                                 \
            ss_ << "CHECK_NEAR failed: " #a " ~ " #b " (" << va_ << " vs " << vb_ << ")";           \
            midiagent_test::reportFailure (__FILE__, __LINE__, ss_.str ());                         \
        }                                                                                           \
    } while (0)
