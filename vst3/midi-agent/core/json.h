// A small JSON value, parser and writer. The composition files of
// midi-agent-skill are plain JSON, and the plug-in must read and write them
// without any dependency outside the C++ standard library.
#pragma once
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace midiagent {

class Json
{
public:
    enum class Type { Null, Bool, Number, String, Array, Object };

    using Array = std::vector<Json>;
    /// Objects keep insertion order so a rewritten file diffs cleanly against
    /// the Python output.
    using Object = std::vector<std::pair<std::string, Json>>;

    Json () = default;
    Json (std::nullptr_t) {}
    Json (bool b) : type_ (Type::Bool), bool_ (b) {}
    Json (int v) : type_ (Type::Number), number_ (v) {}
    Json (int64_t v) : type_ (Type::Number), number_ (static_cast<double> (v)) {}
    Json (double v) : type_ (Type::Number), number_ (v) {}
    Json (const char* s) : type_ (Type::String), string_ (s) {}
    Json (std::string s) : type_ (Type::String), string_ (std::move (s)) {}
    Json (Array a) : type_ (Type::Array), array_ (std::move (a)) {}
    Json (Object o) : type_ (Type::Object), object_ (std::move (o)) {}

    static Json array () { return Json (Array {}); }
    static Json object () { return Json (Object {}); }

    Type type () const { return type_; }
    bool isNull () const { return type_ == Type::Null; }
    bool isBool () const { return type_ == Type::Bool; }
    bool isNumber () const { return type_ == Type::Number; }
    bool isString () const { return type_ == Type::String; }
    bool isArray () const { return type_ == Type::Array; }
    bool isObject () const { return type_ == Type::Object; }

    bool asBool (bool fallback = false) const { return isBool () ? bool_ : fallback; }
    double asNumber (double fallback = 0.0) const { return isNumber () ? number_ : fallback; }
    int asInt (int fallback = 0) const;
    const std::string& asString () const;
    std::string asString (const std::string& fallback) const { return isString () ? string_ : fallback; }
    const Array& asArray () const;
    Array& asArray ();
    const Object& asObject () const;
    Object& asObject ();

    // Object helpers. get () returns a shared null for a missing key.
    bool has (const std::string& key) const;
    const Json& get (const std::string& key) const;
    Json& set (const std::string& key, Json value);
    void erase (const std::string& key);
    // Array helpers.
    void push (Json value);
    size_t size () const;
    const Json& at (size_t index) const;

    /// Parse. On failure returns false and fills `error` with the reason and
    /// the byte offset where parsing stopped.
    static bool parse (const std::string& text, Json& out, std::string& error);
    /// Serialize. indent == 0 writes one line, otherwise pretty prints like
    /// Python's json.dump(indent=2) (the midi-agent-skill convention).
    std::string dump (int indent = 2) const;

private:
    void dumpTo (std::string& out, int indent, int depth) const;

    Type type_ {Type::Null};
    bool bool_ {false};
    double number_ {0.0};
    std::string string_;
    Array array_;
    Object object_;
};

} // namespace midiagent
