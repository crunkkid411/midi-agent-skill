#include "core/json.h"

#include <cmath>
#include <cstdio>
#include <cstring>

namespace midiagent {

namespace {
const Json kNull;
const std::string kEmptyString;
const Json::Array kEmptyArray;
const Json::Object kEmptyObject;

struct Parser
{
    const std::string& s;
    size_t pos {0};
    std::string error;

    explicit Parser (const std::string& text) : s (text) {}

    bool fail (const char* why)
    {
        if (error.empty ())
            error = std::string (why) + " at byte " + std::to_string (pos);
        return false;
    }

    void skipWs ()
    {
        while (pos < s.size () && (s[pos] == ' ' || s[pos] == '\t' || s[pos] == '\n' || s[pos] == '\r'))
            ++pos;
    }

    bool parseValue (Json& out)
    {
        skipWs ();
        if (pos >= s.size ())
            return fail ("unexpected end of input");
        char c = s[pos];
        if (c == '{')
            return parseObject (out);
        if (c == '[')
            return parseArray (out);
        if (c == '"')
        {
            std::string str;
            if (!parseString (str))
                return false;
            out = Json (std::move (str));
            return true;
        }
        if (c == 't' && s.compare (pos, 4, "true") == 0)
        {
            pos += 4;
            out = Json (true);
            return true;
        }
        if (c == 'f' && s.compare (pos, 5, "false") == 0)
        {
            pos += 5;
            out = Json (false);
            return true;
        }
        if (c == 'n' && s.compare (pos, 4, "null") == 0)
        {
            pos += 4;
            out = Json (nullptr);
            return true;
        }
        if (c == '-' || (c >= '0' && c <= '9'))
            return parseNumber (out);
        return fail ("unexpected character");
    }

    bool parseNumber (Json& out)
    {
        size_t start = pos;
        if (s[pos] == '-')
            ++pos;
        while (pos < s.size () && ((s[pos] >= '0' && s[pos] <= '9') || s[pos] == '.' || s[pos] == 'e' ||
                                   s[pos] == 'E' || s[pos] == '+' || s[pos] == '-'))
            ++pos;
        std::string tok = s.substr (start, pos - start);
        char* end = nullptr;
        double v = std::strtod (tok.c_str (), &end);
        if (end == tok.c_str () || *end != '\0')
        {
            pos = start;
            return fail ("bad number");
        }
        out = Json (v);
        return true;
    }

    static void appendUtf8 (std::string& out, uint32_t cp)
    {
        if (cp < 0x80)
            out += static_cast<char> (cp);
        else if (cp < 0x800)
        {
            out += static_cast<char> (0xC0 | (cp >> 6));
            out += static_cast<char> (0x80 | (cp & 0x3F));
        }
        else if (cp < 0x10000)
        {
            out += static_cast<char> (0xE0 | (cp >> 12));
            out += static_cast<char> (0x80 | ((cp >> 6) & 0x3F));
            out += static_cast<char> (0x80 | (cp & 0x3F));
        }
        else
        {
            out += static_cast<char> (0xF0 | (cp >> 18));
            out += static_cast<char> (0x80 | ((cp >> 12) & 0x3F));
            out += static_cast<char> (0x80 | ((cp >> 6) & 0x3F));
            out += static_cast<char> (0x80 | (cp & 0x3F));
        }
    }

    bool parseHex4 (uint32_t& v)
    {
        if (pos + 4 > s.size ())
            return fail ("bad unicode escape");
        v = 0;
        for (int i = 0; i < 4; ++i)
        {
            char c = s[pos++];
            v <<= 4;
            if (c >= '0' && c <= '9')
                v |= static_cast<uint32_t> (c - '0');
            else if (c >= 'a' && c <= 'f')
                v |= static_cast<uint32_t> (c - 'a' + 10);
            else if (c >= 'A' && c <= 'F')
                v |= static_cast<uint32_t> (c - 'A' + 10);
            else
                return fail ("bad unicode escape");
        }
        return true;
    }

    bool parseString (std::string& out)
    {
        ++pos; // opening quote
        while (true)
        {
            if (pos >= s.size ())
                return fail ("unterminated string");
            char c = s[pos++];
            if (c == '"')
                return true;
            if (c == '\\')
            {
                if (pos >= s.size ())
                    return fail ("unterminated escape");
                char e = s[pos++];
                switch (e)
                {
                    case '"': out += '"'; break;
                    case '\\': out += '\\'; break;
                    case '/': out += '/'; break;
                    case 'b': out += '\b'; break;
                    case 'f': out += '\f'; break;
                    case 'n': out += '\n'; break;
                    case 'r': out += '\r'; break;
                    case 't': out += '\t'; break;
                    case 'u':
                    {
                        uint32_t cp = 0;
                        if (!parseHex4 (cp))
                            return false;
                        if (cp >= 0xD800 && cp <= 0xDBFF && pos + 6 <= s.size () && s[pos] == '\\' && s[pos + 1] == 'u')
                        {
                            pos += 2;
                            uint32_t lo = 0;
                            if (!parseHex4 (lo))
                                return false;
                            cp = 0x10000 + ((cp - 0xD800) << 10) + (lo - 0xDC00);
                        }
                        appendUtf8 (out, cp);
                        break;
                    }
                    default: return fail ("bad escape");
                }
            }
            else
                out += c;
        }
    }

    bool parseArray (Json& out)
    {
        ++pos;
        Json::Array items;
        skipWs ();
        if (pos < s.size () && s[pos] == ']')
        {
            ++pos;
            out = Json (std::move (items));
            return true;
        }
        while (true)
        {
            Json v;
            if (!parseValue (v))
                return false;
            items.push_back (std::move (v));
            skipWs ();
            if (pos >= s.size ())
                return fail ("unterminated array");
            if (s[pos] == ',')
            {
                ++pos;
                continue;
            }
            if (s[pos] == ']')
            {
                ++pos;
                out = Json (std::move (items));
                return true;
            }
            return fail ("expected , or ]");
        }
    }

    bool parseObject (Json& out)
    {
        ++pos;
        Json::Object items;
        skipWs ();
        if (pos < s.size () && s[pos] == '}')
        {
            ++pos;
            out = Json (std::move (items));
            return true;
        }
        while (true)
        {
            skipWs ();
            if (pos >= s.size () || s[pos] != '"')
                return fail ("expected object key");
            std::string key;
            if (!parseString (key))
                return false;
            skipWs ();
            if (pos >= s.size () || s[pos] != ':')
                return fail ("expected :");
            ++pos;
            Json v;
            if (!parseValue (v))
                return false;
            items.emplace_back (std::move (key), std::move (v));
            skipWs ();
            if (pos >= s.size ())
                return fail ("unterminated object");
            if (s[pos] == ',')
            {
                ++pos;
                continue;
            }
            if (s[pos] == '}')
            {
                ++pos;
                out = Json (std::move (items));
                return true;
            }
            return fail ("expected , or }");
        }
    }
};

void writeString (std::string& out, const std::string& s)
{
    out += '"';
    for (unsigned char c : s)
    {
        switch (c)
        {
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            case '\b': out += "\\b"; break;
            case '\f': out += "\\f"; break;
            default:
                if (c < 0x20)
                {
                    char buf[8];
                    std::snprintf (buf, sizeof (buf), "\\u%04x", c);
                    out += buf;
                }
                else
                    out += static_cast<char> (c);
        }
    }
    out += '"';
}

void writeNumber (std::string& out, double v)
{
    if (std::isnan (v) || std::isinf (v))
    {
        out += "null";
        return;
    }
    double r = std::round (v);
    if (r == v && std::fabs (v) < 1e15)
    {
        char buf[32];
        std::snprintf (buf, sizeof (buf), "%lld", static_cast<long long> (r));
        out += buf;
        return;
    }
    char buf[32];
    std::snprintf (buf, sizeof (buf), "%.10g", v);
    out += buf;
}
} // namespace

int Json::asInt (int fallback) const
{
    if (!isNumber ())
        return fallback;
    return static_cast<int> (std::lround (number_));
}

const std::string& Json::asString () const
{
    return isString () ? string_ : kEmptyString;
}

const Json::Array& Json::asArray () const
{
    return isArray () ? array_ : kEmptyArray;
}

Json::Array& Json::asArray ()
{
    if (!isArray ())
    {
        type_ = Type::Array;
        array_.clear ();
    }
    return array_;
}

const Json::Object& Json::asObject () const
{
    return isObject () ? object_ : kEmptyObject;
}

Json::Object& Json::asObject ()
{
    if (!isObject ())
    {
        type_ = Type::Object;
        object_.clear ();
    }
    return object_;
}

bool Json::has (const std::string& key) const
{
    if (!isObject ())
        return false;
    for (const auto& kv : object_)
        if (kv.first == key)
            return true;
    return false;
}

const Json& Json::get (const std::string& key) const
{
    if (!isObject ())
        return kNull;
    for (const auto& kv : object_)
        if (kv.first == key)
            return kv.second;
    return kNull;
}

Json& Json::set (const std::string& key, Json value)
{
    auto& obj = asObject ();
    for (auto& kv : obj)
        if (kv.first == key)
        {
            kv.second = std::move (value);
            return kv.second;
        }
    obj.emplace_back (key, std::move (value));
    return obj.back ().second;
}

void Json::erase (const std::string& key)
{
    if (!isObject ())
        return;
    for (auto it = object_.begin (); it != object_.end (); ++it)
        if (it->first == key)
        {
            object_.erase (it);
            return;
        }
}

void Json::push (Json value)
{
    asArray ().push_back (std::move (value));
}

size_t Json::size () const
{
    if (isArray ())
        return array_.size ();
    if (isObject ())
        return object_.size ();
    return 0;
}

const Json& Json::at (size_t index) const
{
    if (!isArray () || index >= array_.size ())
        return kNull;
    return array_[index];
}

bool Json::parse (const std::string& text, Json& out, std::string& error)
{
    Parser p (text);
    Json v;
    if (!p.parseValue (v))
    {
        error = p.error;
        return false;
    }
    p.skipWs ();
    if (p.pos != text.size ())
    {
        p.fail ("trailing characters");
        error = p.error;
        return false;
    }
    out = std::move (v);
    error.clear ();
    return true;
}

std::string Json::dump (int indent) const
{
    std::string out;
    dumpTo (out, indent, 0);
    return out;
}

void Json::dumpTo (std::string& out, int indent, int depth) const
{
    auto newline = [&] (int d) {
        if (indent <= 0)
            return;
        out += '\n';
        out.append (static_cast<size_t> (d * indent), ' ');
    };
    switch (type_)
    {
        case Type::Null: out += "null"; break;
        case Type::Bool: out += bool_ ? "true" : "false"; break;
        case Type::Number: writeNumber (out, number_); break;
        case Type::String: writeString (out, string_); break;
        case Type::Array:
            if (array_.empty ())
            {
                out += "[]";
                break;
            }
            out += '[';
            for (size_t i = 0; i < array_.size (); ++i)
            {
                if (i)
                    out += ',';
                newline (depth + 1);
                array_[i].dumpTo (out, indent, depth + 1);
            }
            newline (depth);
            out += ']';
            break;
        case Type::Object:
            if (object_.empty ())
            {
                out += "{}";
                break;
            }
            out += '{';
            for (size_t i = 0; i < object_.size (); ++i)
            {
                if (i)
                    out += ',';
                newline (depth + 1);
                writeString (out, object_[i].first);
                out += indent > 0 ? ": " : ":";
                object_[i].second.dumpTo (out, indent, depth + 1);
            }
            newline (depth);
            out += '}';
            break;
    }
}

} // namespace midiagent
