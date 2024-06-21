/*
Single header json & json5 parser.
Json5 1.0.0 compliant.

---

Include as shown below in exactly 1 of your cpp files to generate the implementation:

#define TT_JSON5_IMPLEMENTATION
#include "tt_json5.h"

Include regularly in other files.

The following preprocessor configuration options are supported:

// Use wchar_t and std::wstring, benchmarks show that this is the fastest option.
#define TT_JSON5_USE_WSTR

// Use 128 bit scalars.
#define TT_JSON5_LONG_DOUBLE

// Use 32 bit scalars.
#define TT_JSON5_NO_DOUBLE

// Disable json5 support, benchmarks show that fewer features is somehow slower.
#define TT_JSON5_NO_JSON5

In addition, inside tt_json5.h individual json5 features can be turned off at the top of the file.
Each json5 feature has its own define, when all defines are disabled, the parser reverts to a regular json parser.
---

MIT License

Copyright (c) 2021 trevor van hoof

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <codecvt>
#include <functional>
#include <unordered_map>
#include "windont.h"
#include <stringapiset.h>

#ifndef TT_JSON5_NO_JSON5
// Individual JSON5 features (turning them all off reverts this to a strict JSON compliant parser):
// This turned out to be quite the rabbit hole as it defines many rules around which unicode subsets are allowed.
// Because we don't have a regular expression engine that can filter by unicode group this ends up with a series of huge tables to check for
// (it could be optimized into a list of valid ranges but even that is a very large table).
// So instead, we'll implement this badly by searching for all json control characters instead!
// If we are at the point of parsing a key, we'll consume all non-whitespace as long as it is not one of ":},".
// We need } because of trailing comma logic. Imagine "{a: false,}". We will parse a key after the comma, 
// if we detect } as the name of a key, our parser breaks too late.
#define TT_JSON5_OBJECT_SUPPORT_IDENTIFIER_NAMES_KEYS // Allow keys to omit quotes and instead adhere to ECMA5.1 identifier name rules.

#define TT_JSON5_OBJECT_SUPPORT_TRAILING_COMMA // Allow trailing comma in object. Note that lone comma or leading comma are not allowed: {,} or {,"k":<v>}

#define TT_JSON5_ARRAY_SUPPORT_TRAILING_COMMA // Allow trailing comma in array. Note that lone comma or leading comma are not allowed: [,] or [,<v>]

#define TT_JSON5_STRING_SUPPORT_SINGLE_QUOTES // Allow single quoted strings. End quote must match start quote. Escape quote only works on matching quotes, else it is a parse error. This is consistent with original json behavior: "\'" is an error.
#define TT_JSON5_STRING_SUPPORT_ESCAPE_LINE_BREAKS // Allow multiline strings by putting \ at the end of a line.
#define TT_JSON5_STRING_SUPPORT_HEX  // Allow \xFF style escape codes besides the regular \u9999 style escape codes
#define TT_JSON5_STRING_SUPPORT_CHARACTER_ESCAPES // \ just ignores the next character, no matter what it is. Not sure if I got the spec right on this.

#define TT_JSON5_NUMBER_SUPPORT_HEX // Allow integers to be entered as hex, e.g. 0xD00F.
#define TT_JSON5_NUMBER_SUPPPORT_FREE_DECIMAL // Allow floating decimal points so .0 and 0. become valid numbers.
#define TT_JSON5_NUMBER_SUPPORT_INF_AND_NAN  // Allow case-sensitive keywords NaN, Infinity and -Infinity.
#define TT_JSON5_NUMBER_SUPPORT_PLUS_SIGN // Allow explicit + sign in front of numbers (including +Infinity if Infinity support is enabled).

#define TT_JSON5_SUPPORT_SINGLE_LINE_COMMENTS 
#define TT_JSON5_SUPPORT_BLOCK_COMMENTS 

// By default, whitespace skipping calls are only in places where the json spec allows it.
#define TT_JSON5_SUPPORT_MORE_WHITESPACE // Enable more whitespace skipping calls, because in our implementation comments are treated as whitespace, this is a must when allowing comments.
#endif

namespace TTJson {
#ifdef TT_JSON5_USE_WSTR
    typedef std::wifstream ifstream_t;
    typedef std::wistream istream_t;
    typedef std::wofstream ofstream_t;
    typedef std::wostream ostream_t;
    typedef std::wstring str_t;
    typedef std::wstringstream sstr_t;
    typedef wchar_t char_t;

    str_t makeString(const char* c);
    str_t makeString(char c);
#else
    typedef std::ifstream ifstream_t;
    typedef std::istream istream_t;
    typedef std::ofstream ofstream_t;
    typedef std::ostream ostream_t;
    typedef std::string str_t;
    typedef std::stringstream sstr_t;
    typedef char char_t;

    str_t makeString(const wchar_t* c);
    str_t makeString(wchar_t c);
#endif

    // Utility function to write either makeString(L"") or makeString("") and get the right string type back.
    // When using wstrings, single byte literals are assumed to be UTF8 encoded.
    str_t makeString(const char_t* c);
    str_t makeString(char_t c);

    // Select scalar type.
#ifdef TT_JSON5_LONG_DOUBLE
    typedef long double scalar;
#elif defined(TT_JSON5_NO_DOUBLE)
    typedef float scalar;
#else
    typedef double scalar;
#endif

    enum class ValueType {
        Bool,
        Int,
        Double,
        String,
        Array,
        Object,
        Null
    };

    class Value;

    class Array : public std::vector<Value> {
    public:
        using vector::vector;

        Value& operator[](size_t index);
        const Value& operator[](size_t index) const;
    };

    class Object : public std::unordered_map<str_t, Value> {
    public:
        using unordered_map::unordered_map;

        Value& get(const str_t& key);
        const Value& get(const str_t& key) const;

        const Value* tryGet(const str_t& key) const;
        const bool* tryGetBool(const str_t& key) const;
        const long long* tryGetInt(const str_t& key) const;
        const double* tryGetDouble(const str_t& key) const;
        const str_t* tryGetString(const str_t& key) const;
        const Array* tryGetArray(const str_t& key) const;
        const Object* tryGetObject(const str_t& key) const;
    };

    class Value {
        friend class Parser;
        friend void serialize(const Value&, ostream_t&, const char_t*, int);

        ValueType type;

        bool bValue{};
        long long iValue{};
        scalar dValue{};
        str_t sValue{};
        Array aValue{};
        Object oValue{};

        typedef void (*errorFunc)();

    public:
        Value(ValueType type = ValueType::Null);
        Value(bool value);
        Value(long long value);
        Value(str_t value);
        Value(const Array& value);
        Value(const Object & value);
        Value(float value);
        Value(double value);
        Value(long double value);

        static errorFunc castErrorHandler;

        const bool& asBool() const;
        const long long& asInt() const;
        const scalar& asDouble() const;
        const str_t& asString() const;
        const Array& asArray() const;
        const Object& asObject() const;

        bool& asBool();
        long long& asInt();
        scalar& asDouble();
        str_t& asString();
        Array& asArray();
        Object& asObject();

        bool isNull() const;
        bool isBool() const;
        bool isInt() const;
        bool isDouble() const;
        bool isString() const;
        bool isArray() const;
        bool isObject() const;
    };

    class Parser {
        str_t parseError{};
        int errorCode = 0;
        size_t cursor = 0;
        size_t lineNumber = 0;
        size_t columnNumber = 0;
        size_t prevColumnNumber = 0;
        // Currently necessary to avoid the use of unget, as it causes a crash in the unit tests.
        bool rewind = false; 
        char_t lastChr = '\0';

        inline void clearError();
        inline void throwNotImplementedError(const str_t& msg = {});
        inline void throwParseError(const str_t& msg = {});
        inline void throwReadError(const str_t& msg = {});
        inline void throwRewindError(const str_t& msg = {});
        inline void throwEOF(const str_t& msg = {});

        char_t peek1(istream_t& stream);
        char_t read1(istream_t& stream);
        void rewind1(istream_t& stream);
#if defined(TT_JSON5_SUPPORT_BLOCK_COMMENTS) || defined(TT_JSON5_SUPPORT_SINGLE_LINE_COMMENTS)
        bool skipComments(istream_t& stream, char_t& b);
#endif
        void skipWhitespace(istream_t& stream);
        static void writeUTF8(sstr_t& dst, unsigned short codePoint);
        // -1 is failure, rewinds and does not throw
        int readHexChar(istream_t& stream);

        bool parseKeyword(istream_t& stream, const str_t& word, bool rewindOnFail = true);
#ifdef TT_JSON5_STRING_SUPPORT_SINGLE_QUOTES
        str_t parseString(istream_t& stream, const char_t closingQuote = '"');
#else
        str_t parseString(istream_t& stream);
#endif

        void parseNumber(istream_t& stream, char_t first, Value& result);
        str_t parseKey(istream_t& stream);
        void parseObject(istream_t& stream, Value& result);
        void parseArray(istream_t& stream, Value& result);
        void parseValue(istream_t& stream, Value& result);

    public:
        bool hasError();
        str_t error();
        void parse(istream_t& stream, Value& result);
    };

    // Utilities to open fstreams with utf8 encoding.
    ifstream_t readUtf8(const std::string& path);
    ofstream_t writeUtf8(const std::string & path);

    void serialize(const Value& value, ostream_t& out, const char_t* tab = nullptr, int depth = 0);
}

#ifdef TT_JSON5_IMPLEMENTATION
namespace TTJson {
#ifdef TT_JSON5_USE_WSTR
    str_t makeString(const char* c) {
        const size_t cSize = strlen(c);
        std::wstring wc(cSize, L'\0');
        mbstowcs_s(0, &wc[0], cSize + 1, c, cSize);
        return wc;
    }
    str_t makeString(const char c) {
        std::wstring wc(1, L'\0');
        char buf[1];
        buf[0] = c;
        mbstowcs_s(0, &wc[0], 2, buf, 1);
        return wc;
    }
#else
    str_t makeString(const wchar_t* c) {
        size_t sz = wcslen(c);
        str_t r;
        r.resize(sz * 4, '\0');
        WideCharToMultiByte(CP_UTF8, 0, c, (int)sz, r.data(), (int)r.size(), nullptr, nullptr);
        return r;
    }
    str_t makeString(wchar_t c) {
        str_t r;
        r.resize(4, '\0');
        WideCharToMultiByte(CP_UTF8, 0, &c, 1, r.data(), (int)r.size(), nullptr, nullptr);
        return r;
    }
#endif
    str_t makeString(const char_t* c) {
        return c;
    }
    str_t makeString(char_t c) {
        return { c };
    }

    namespace {
        Value _INVALID{};
    }

    Value& Array::operator[](size_t index) {
        if (index >= size()) return _INVALID;
        return std::vector<Value>::operator[](index);
    }

    const Value& Array::operator[](size_t index) const {
        if (index >= size()) return _INVALID;
        return std::vector<Value>::operator[](index);
    }

    Value& Object::get(const str_t& key) {
        auto it = find(key);
        if (find(key) == end()) return _INVALID;
        return it->second;
    }

    const Value& Object::get(const str_t& key) const {
        auto it = find(key);
        if (find(key) == end()) return _INVALID;
        return it->second;
    }

    const Value* Object::tryGet(const str_t& key) const {
        auto it = find(key);
        if (it != end()) return &it->second;
        return nullptr;
    }

    const bool* Object::tryGetBool(const str_t& key) const {
        const Value* value = tryGet(key);
        if (value && value->isBool()) return &value->asBool();
        return nullptr;
    }

    const long long* Object::tryGetInt(const str_t& key) const {
        const Value* value = tryGet(key);
        if (value && value->isInt()) return &value->asInt();
        return nullptr;
    }

    const double* Object::tryGetDouble(const str_t& key) const {
        const Value* value = tryGet(key);
        if (value && value->isDouble()) return &value->asDouble();
        return nullptr;
    }

    const str_t* Object::tryGetString(const str_t& key) const {
        const Value* value = tryGet(key);
        if (value && value->isString()) return &value->asString();
        return nullptr;
    }

    const Array* Object::tryGetArray(const str_t& key) const {
        const Value* value = tryGet(key);
        if (value && value->isArray()) return &value->asArray();
        return nullptr;
    }

    const Object* Object::tryGetObject(const str_t& key) const {
        const Value* value = tryGet(key);
        if (value && value->isObject()) return &value->asObject();
        return nullptr;
    }

    Value::errorFunc Value::castErrorHandler = nullptr;

    Value::Value(ValueType type) : type(type) {}
    Value::Value(bool value) : type(ValueType::Bool), bValue(value) {}
    Value::Value(long long value) : type(ValueType::Int), iValue(value) {}
    Value::Value(str_t value) : type(ValueType::String), sValue(value) {}
    Value::Value(const Array& value) : type(ValueType::Array), aValue(value) {}
    Value::Value(const Object& value) : type(ValueType::Object), oValue(value) {}
    Value::Value(float value) : type(ValueType::Double), dValue(value) {}
    Value::Value(double value) : type(ValueType::Double), dValue(value) {}
    Value::Value(long double value) : type(ValueType::Double), dValue(value) {}

    const bool& Value::asBool() const { if (type != ValueType::Bool && castErrorHandler != nullptr) castErrorHandler(); return bValue; }
    const long long& Value::asInt() const { if (type != ValueType::Int && castErrorHandler != nullptr) castErrorHandler(); return iValue; }
    const scalar& Value::asDouble() const { if (type != ValueType::Double && castErrorHandler != nullptr) castErrorHandler(); return dValue; }
    const str_t& Value::asString() const { if (type != ValueType::String && castErrorHandler != nullptr) castErrorHandler(); return sValue; }
    const Array& Value::asArray() const { if (type != ValueType::Array && castErrorHandler != nullptr) castErrorHandler(); return aValue; }
    const Object& Value::asObject() const { if (type != ValueType::Object && castErrorHandler != nullptr) castErrorHandler(); return oValue; }

    bool& Value::asBool() { if (type != ValueType::Bool && castErrorHandler != nullptr) castErrorHandler(); return bValue; }
    long long& Value::asInt() { if (type != ValueType::Int && castErrorHandler != nullptr) castErrorHandler(); return iValue; }
    scalar& Value::asDouble() { if (type != ValueType::Double && castErrorHandler != nullptr) castErrorHandler(); return dValue; }
    str_t& Value::asString() { if (type != ValueType::String && castErrorHandler != nullptr) castErrorHandler(); return sValue; }
    Array& Value::asArray() { if (type != ValueType::Array && castErrorHandler != nullptr) castErrorHandler(); return aValue; }
    Object& Value::asObject() { if (type != ValueType::Object && castErrorHandler != nullptr) castErrorHandler(); return oValue; }

    bool Value::isNull() const { return type == ValueType::Null; }
    bool Value::isBool() const { return type == ValueType::Bool; }
    bool Value::isInt() const { return type == ValueType::Int; }
    bool Value::isDouble() const { return type == ValueType::Double; }
    bool Value::isString() const { return type == ValueType::String; }
    bool Value::isArray() const { return type == ValueType::Array; }
    bool Value::isObject() const { return type == ValueType::Object; }

    inline void Parser::clearError() {
        parseError.clear();
        errorCode = 0;
    }

    inline void Parser::throwNotImplementedError(const str_t& msg) {
        parseError = msg;
        errorCode = 1;
    }

    inline void Parser::throwParseError(const str_t& msg) {
        parseError = msg;
        errorCode = 2;
    }

    inline void Parser::throwReadError(const str_t& msg) {
        parseError = msg;
        errorCode = 3;
    }

    inline void Parser::throwRewindError(const str_t& msg) {
        parseError = msg;
        errorCode = 4;

    }

    inline void Parser::throwEOF(const str_t& msg) {
        parseError = msg;
        errorCode = 5;
    }

    char_t Parser::peek1(istream_t& stream) {
        char_t result = stream.peek();

        if (stream.eof()) {
            throwEOF();
            return '\0';
        }

        if (!stream.good()) {
            throwReadError();
            return '\0';
        }

        return result;
    }

    char_t Parser::read1(istream_t& stream) {
        if (rewind) {
            rewind = false;
            return lastChr;
        }

        stream.read(&lastChr, 1);

        if (stream.eof()) {
            throwEOF();
            return '\0';
        }

        if (!stream.good()) {
            throwReadError();
            return '\0';
        }

        ++cursor;
        ++columnNumber;
        if (lastChr == '\n') {
            ++lineNumber;
            prevColumnNumber = columnNumber;
            columnNumber = 0;
        }

        return lastChr;
    }

    void Parser::rewind1(istream_t& stream) {
        if (rewind)
            throwRewindError();
        rewind = true;
    }

#if defined(TT_JSON5_SUPPORT_BLOCK_COMMENTS) || defined(TT_JSON5_SUPPORT_SINGLE_LINE_COMMENTS)
    bool Parser::skipComments(istream_t& stream, char_t& b) {
        if (b != '/')
            return false;
        b = read1(stream);
#ifdef TT_JSON5_SUPPORT_BLOCK_COMMENTS
        bool block = b == '*';
#else
        bool block = false;
#endif
#ifdef TT_JSON5_SUPPORT_SINGLE_LINE_COMMENTS
        bool line = b == '/';
#else
        bool line = false;
#endif
        if (!line && !block) {
            rewind1(stream);
            return false;
        }

        b = read1(stream);
        bool escape = false;
        while (true) {
            if (escape) {
                escape = false;
                b = read1(stream);
                continue;
            }
            if (stream.eof()) {
                if (!block)
                    clearError();
                return true;
            }
            if (b == '\\') {
                escape = true;
                continue;
            }
            if (!block && (b == '\n' || b == '\r')) {
                break;
            }
            if (block && b == '*') {
                b = read1(stream);
                if (b == '/')
                    break;
                else
                    continue;
                break;
            }
            b = read1(stream);
        }
        b = read1(stream);
        if (stream.eof()) {
            clearError();
            return true;
        }
        return false;
    }
#endif

    void Parser::skipWhitespace(istream_t& stream) {
        if (errorCode != 0) return;

        char_t chr = read1(stream);
        if (errorCode != 0) { clearError(); return; }

#if defined(TT_JSON5_SUPPORT_BLOCK_COMMENTS) || defined(TT_JSON5_SUPPORT_SINGLE_LINE_COMMENTS)
        // if chr is the start of a comment, updates chr with the next chr after the comment
        // returns true if EOF directly after end of comment
        if (skipComments(stream, chr))
            return;
        if (errorCode != 0) return;
#endif

        while (chr == ' ' || chr == '\r' || chr == '\n' || chr == '\x0c') {
            chr = read1(stream);
            if (errorCode != 0) { clearError(); return; }

#if defined(TT_JSON5_SUPPORT_BLOCK_COMMENTS) || defined(TT_JSON5_SUPPORT_SINGLE_LINE_COMMENTS)
            // if chr is the start of a comment, updates chr with the next chr after the comment
            // returns true if EOF directly after end of comment
            if (skipComments(stream, chr))
                return;
            if (errorCode != 0) return;
#endif
        }

        rewind1(stream);
    }

    bool Parser::parseKeyword(istream_t& stream, const str_t& word, bool rewindOnFail) {
        if (errorCode != 0)
            return false;
        size_t i = 0;
        size_t n = word.size();
        for (; i < word.size(); ++i) {
            char_t chr = read1(stream);
            if (errorCode != 0)
                return false;
            if (chr != word[i]) {
                if (rewindOnFail) {
                    while (i) {
                        // TODO: The empty array test case broke on unget so I no longer trust it. Can we work around this?
                        stream.unget();
                        --i;
                    }
                } else {
                    throwParseError(makeString("Expected '") + word[i] + makeString("' instead of '") + chr + makeString("'."));
                    return false;
                }
                return false;
            }
        }
        return true;
    }

    void Parser::writeUTF8(sstr_t& dst, unsigned short codePoint)
    {
#ifdef TT_JSON5_USE_WSTR
        dst << *reinterpret_cast<wchar_t*>(&codePoint);
#else
        static char buf[5];
        memset(buf, 0, sizeof(buf));
        WideCharToMultiByte(CP_UTF8, 0, reinterpret_cast<wchar_t*>(&codePoint), 1, buf, sizeof(buf), nullptr, nullptr);
        dst << buf;
#endif
    }

#ifdef TT_JSON5_STRING_SUPPORT_SINGLE_QUOTES
    str_t Parser::parseString(istream_t& stream, const char_t closingQuote) {
#else
    str_t Parser::parseString(istream_t & stream) {
        const char_t closingQuote = '"';
#endif
        if (errorCode != 0) return {};
        char_t lead = read1(stream);
        if (errorCode != 0) return {};
        sstr_t str{};
        bool escape = false;
        while (true) {
            if (!escape) {
                if (lead == '\\')
                    escape = true;
                else if (lead == closingQuote)
                    break;
                else {
                    if (lead == '\r' || lead == '\n') {
                        throwParseError(makeString("Unexpected line break."));
                        return {};
                    }
                    str << lead;
                }
            } else if (lead == 'u') {
                // verify unicode escape char
                unsigned short codePoint = 0;
                for (int i = 0; i < 4; ++i) {
                    int value = readHexChar(stream);
                    if (value == -1) {
                        throwParseError(makeString("Invalid unicode escape code, expected 4 hexadecimal digits, not ") + peek1(stream) + makeString('.'));
                        return {};
                    }
                    codePoint <<= 4;
                    codePoint += value;
                }
                writeUTF8(str, codePoint);
#ifdef TT_JSON5_STRING_SUPPORT_HEX
            } else if (lead == 'x') {
                // verify hex escape char
                char_t codePoint = 0;
                for (int i = 0; i < 2; ++i) {
                    int value = readHexChar(stream);
                    if (value == -1) {
                        throwParseError(makeString("Invalid hex escape code, expected 2 hexadecimal digits, not ") + peek1(stream) + makeString('.'));
                        return {};
                    }
                    codePoint <<= 4;
                    codePoint += value;
                }
                str << codePoint;
#endif
#ifdef TT_JSON5_STRING_SUPPORT_ESCAPE_LINE_BREAKS 
                // TODO: Include U+2028 and U+2029 here as well
            } else if (lead == '\r' || lead == '\n') {
                if (lead == '\r') { // Windows
                    lead = read1(stream);
                    if (lead != '\n')
                        rewind1(stream);
                    escape = false;
                }
#endif
            } else {
#ifndef TT_JSON5_STRING_SUPPORT_CHARACTER_ESCAPES
                if (lead != closingQuote &&
                    lead != '\\' &&
                    lead != '/' &&
                    lead != 'b' &&
                    lead != 'f' &&
                    lead != 'n' &&
                    lead != 'r' &&
                    lead != 't') {
                    throwParseError(makeString("Invalid escape sequence \\") + lead + makeString('.'));
                    return {};
                }
#endif
                str << '\\';
                str << lead;
                escape = false;
            }
            lead = read1(stream);
            if (errorCode != 0) return {};
        }

        return str.str();
    }

    // -1 is failure, rewinds and does not throw
    int Parser::readHexChar(istream_t& stream) {
        char_t hex = read1(stream);
        if (hex >= '0' && hex <= '9')
            return hex - '0';
        else if (hex >= 'a' && hex <= 'f')
            return hex - 'a' + 10;
        else if (hex >= 'A' && hex <= 'F')
            return hex - 'A' + 10;
        rewind1(stream);
        return -1;
    }

    void Parser::parseNumber(istream_t& stream, char_t first, Value& result) {
#ifdef TT_JSON5_NUMBER_SUPPORT_INF_AND_NAN
        if (first == 'N') {
            parseKeyword(stream, makeString("aN"), false);
            if (errorCode != 0) return;
            result.type = ValueType::Double;
            result.dValue = std::numeric_limits<scalar>::quiet_NaN();
            return;
        }
#endif

        bool negative = first == '-';

        char_t b = first;
#ifdef TT_JSON5_NUMBER_SUPPORT_PLUS_SIGN
        if (negative || first == '+')
#else
        if (negative)
#endif
            b = read1(stream);

#ifdef TT_JSON5_NUMBER_SUPPORT_INF_AND_NAN
        if (b == 'I') {
            parseKeyword(stream, makeString("nfinity"), false);
            if (errorCode != 0) return;
            result.type = ValueType::Double;
            result.dValue = std::numeric_limits<scalar>::infinity();
            if (negative)
                result.dValue = -result.dValue;
            return;
        }
#endif

#ifdef TT_JSON5_NUMBER_SUPPORT_HEX
        if (b == '0') {
            char_t tmp = peek1(stream);
            if (errorCode != 0)
                return;
            if (tmp == 'x' || tmp == 'X') {
                read1(stream); // actually consume the x
                result.type = ValueType::Int;
                bool haveData = false;
                while (true) {
                    result.iValue <<= 4;
                    int v = readHexChar(stream);
                    if (v == -1) {
                        if (!haveData) {
                            throwParseError(makeString("Unexpected '") + peek1(stream) + makeString("', expected hexadecimal digit."));
                            return;
                        }
                        break;
                    }
                    result.iValue += v;
                    haveData = true;
                }
                return;
            }
        }
#endif

        str_t head{};
        str_t tail{};
        str_t exponent{};
        bool exponentNegative = false;

        enum class Mode {
            AFTER_SIGN = 0,
            HEAD = 1,
            AFTER_HEAD = 2,
            FRACTION = 3,
            SEARCH_EXPONENT = 4,
            EXPONENT_SIGN = 5,
            EXPONENT_DIGIT = 6,
            FINISHED = 7,
        };

        Mode mode = Mode::AFTER_SIGN;
        while (true) {
            if (mode == Mode::AFTER_SIGN) {
                // We can find 1 optional 1-9, or a 0
                if (b >= '1' && b <= '9') {
                    mode = Mode::HEAD;
                    head += b;
                } else if (b == '0') {
                    mode = Mode::AFTER_HEAD;
                    head += b;
                } else {
#ifdef TT_JSON5_NUMBER_SUPPPORT_FREE_DECIMAL
                    mode = Mode::AFTER_HEAD;
                    continue;
#else
                    throwParseError();
                    return;
#endif
                }
            } else if (mode == Mode::HEAD) {
                // We can find more optional digits, or move on
                if (b >= '0' && b <= '9')
                    head += b;
                else {
                    mode = Mode::AFTER_HEAD;
                    continue;
                }
            } else if (mode == Mode::AFTER_HEAD) {
                // We can find an optional dot, or move on
                if (b == '.')
                    mode = Mode::FRACTION;
                else {
                    // End of number
                    if (head.empty()) {
                        throwParseError();
                        return;
                    }
                    mode = Mode::SEARCH_EXPONENT;
                    continue;
                }
            } else if (mode == Mode::FRACTION) {
                // We can find 1 or more digits, or move on
                if (b >= '0' && b <= '9')
                    tail += b;
                else {
#ifndef TT_JSON5_NUMBER_SUPPPORT_FREE_DECIMAL
                    if (tail.empty()) {
                        throwParseError();
                        return;
                    }
#endif
                    mode = Mode::SEARCH_EXPONENT;
                    continue;
                }
            } else if (mode == Mode::SEARCH_EXPONENT) {
                // We can find an exponent, or we are done
                if (b == 'e' || b == 'E')
                    mode = Mode::EXPONENT_SIGN;
                else
                    break;
            } else if (mode == Mode::EXPONENT_SIGN) {
                // We can find an optional sign, and move on
                if (b == '+' || b == '-')
                    exponentNegative = b == '-';
                else {
                    mode = Mode::EXPONENT_DIGIT;
                    continue;
                }
            } else if (mode == Mode::EXPONENT_DIGIT) {
                // We can find 1 or more digits, or move on
                if (b == '.') {
                    throwParseError();
                    return;
                }
                if (b >= '0' && b <= '9')
                    exponent += b;
                else {
                    if (exponent.empty()) {
                        throwParseError();
                        return;
                    }
                    mode = Mode::FINISHED;
                    break;
                }
            }
            // Get next byte
            b = read1(stream);
            if (errorCode != 0) return;
        }

        rewind1(stream);

#ifndef TT_JSON5_NUMBER_SUPPPORT_FREE_DECIMAL
        if (head.empty() || (mode == Mode::FRACTION && tail.empty()) || (mode == Mode::EXPONENT_DIGIT && exponent.empty())) {
            throwParseError();
            return;
        }
#else
        if ((head.empty() && tail.empty()) || (mode == Mode::EXPONENT_DIGIT && exponent.empty())) {
            throwParseError();
            return;
        }
#endif

        if (negative) {
            head = makeString('-') + head;
        }

        if (!tail.empty()) {
            head += '.';
            head += tail;
        }

        if (!exponent.empty()) {
            head += 'e';
            if (exponentNegative) {
                head += '-';
            }
            head += exponent;
        }

        if (tail.empty() && exponent.empty() && mode != Mode::FRACTION) {
            result.iValue = stoll(head);
            result.type = ValueType::Int;
        } else {
            result.dValue = stold(head);
            result.type = ValueType::Double;
        }
    }

    str_t Parser::parseKey(istream_t& stream) {
        char_t lead = read1(stream);
        if (errorCode != 0) return {};

        if (lead != '"') {
#ifdef TT_JSON5_OBJECT_SUPPORT_IDENTIFIER_NAMES_KEYS
            const str_t sentinels = makeString(":},");
            sstr_t str{};
            bool escape = false;
            while (true) {
                if (!escape) {
                    if (lead == '\\')
                        escape = true;
                    else if (sentinels.find(lead) != str_t::npos)
                        break;
                    else
                        str << lead;
                } else if (lead == 'u') {
                    // verify unicode escape char
                    unsigned short codePoint = 0;
                    for (int i = 0; i < 4; ++i) {
                        int value = readHexChar(stream);
                        if (value == -1) {
                            throwParseError(makeString("Invalid unicode escape code, expected 4 hexadecimal digits, not ") + peek1(stream) + makeString('.'));
                            return {};
                        }
                        codePoint <<= 4;
                        codePoint += value;
                    }
                    writeUTF8(str, codePoint);
                    escape = false;
                } else {
                    throwParseError(makeString("Unexpected '\\'."));
                    return {};
                }
                lead = read1(stream);
                if (errorCode != 0) return {};
            }
            rewind1(stream);
            return str.str();
#else
            throwParseError(makeString("Expected '\"' instead of '") + lead + makeString("'."));
            return {};
#endif
        }

        return parseString(stream);
    }

    void Parser::parseObject(istream_t& stream, Value& result) {
        result.type = ValueType::Object;

        skipWhitespace(stream);
        if (errorCode != 0) return;
#ifndef TT_JSON5_OBJECT_SUPPORT_TRAILING_COMMA
        if (read1(stream) == '}')
            return;
        rewind1(stream);
        if (errorCode != 0) return;
#endif

        while (true) {
#ifdef TT_JSON5_OBJECT_SUPPORT_TRAILING_COMMA
            // If we reach here on the first loop, we have {}
            // If we reach here on subsequent loops, we have {"k":<v>,}
            char_t lead = read1(stream);
            if (lead == '}')
                return;
            else
                rewind1(stream);
#endif
            Value& element = result.oValue[parseKey(stream)];
            // if (errorCode != 0) return;

            skipWhitespace(stream);
            if (errorCode != 0) return;

            char_t delim = read1(stream);
            if (errorCode != 0) return;
            if (delim != ':') {
                throwParseError(makeString("Expected ':' instead of '") + delim + makeString("'."));
                return;
            }

            parseValue(stream, element);

            char_t comma = read1(stream);
            if (comma != ',') {
                if (comma != '}') {
                    throwParseError(makeString("Expected '}' instead of '") + comma + makeString("'."));
                    return;
                }
                break;
            }
            skipWhitespace(stream);
        }
    }

    void Parser::parseArray(istream_t& stream, Value& result) {
        result.type = ValueType::Array;

        skipWhitespace(stream);
        if (errorCode != 0) return;
#ifndef TT_JSON5_ARRAY_SUPPORT_TRAILING_COMMA
        if (read1(stream) == ']')
            return;
        rewind1(stream);
        if (errorCode != 0) return;
#endif

        while (true) {
#ifdef TT_JSON5_ARRAY_SUPPORT_TRAILING_COMMA
            skipWhitespace(stream);
            if (errorCode != 0) return;

            // If we reach here on the first loop, we have []
            // If we reach here on subsequent loops, we have [value,]
            char_t lead = read1(stream);
            // if (errorCode != 0) return;
            if (lead == ']')
                return;
            else
                rewind1(stream);
#endif
            result.aValue.emplace_back();
            parseValue(stream, result.aValue.back());
            if (errorCode != 0) return;

            char_t comma = read1(stream);
            if (errorCode != 0) return;
            if (comma != ',') {
                if (comma != ']') {
                    throwParseError(makeString("Expected ']' instead of '") + comma + makeString("'."));
                    return;
                }
                break;
            }
        }
    }

    void Parser::parseValue(istream_t& stream, Value& result) {
        if (errorCode != 0) return;

        skipWhitespace(stream);

        char_t lead = read1(stream);
        if (errorCode != 0) return;

        if (lead == '{')
            parseObject(stream, result);
        else if (lead == '[')
            parseArray(stream, result);
        else if (lead == '"') {
            result.type = ValueType::String;
            result.sValue = parseString(stream);
#ifdef TT_JSON5_STRING_SUPPORT_SINGLE_QUOTES
        } else if (lead == '\'') {
            result.type = ValueType::String;
            result.sValue = parseString(stream, '\'');
#endif
        } else if (lead == 'f' && parseKeyword(stream, makeString("alse"))) {
            result.type = ValueType::Bool;
            result.bValue = false;
        } else if (lead == 't' && parseKeyword(stream, makeString("rue"))) {
            result.type = ValueType::Bool;
            result.bValue = true;
        } else if (lead == 'n' && parseKeyword(stream, makeString("ull")))
            result.type = ValueType::Null;
        else {
            parseNumber(stream, lead, result);
        }

        skipWhitespace(stream);
    }

    bool Parser::hasError() {
        return errorCode != 0;
    }

    str_t Parser::error() {
        str_t result{};
        switch (errorCode) {
        case 1:
            parseError += makeString("Not implemented error at: ");
            break;
        case 2:
            parseError += makeString("Parse error at: ");
            break;
        case 3:
            parseError += makeString("Unexpected read error at: ");
            break;
        case 4:
            parseError += makeString("Unexpected rewind error at: ");
            break;
        case 5:
            parseError += makeString("Unexpected EOF at: ");
            break;
        }
#ifdef TT_JSON5_USE_WSTR
        result += std::to_wstring(cursor);
#else
        result += std::to_string(cursor);
#endif
        result += makeString(", line: ");
#ifdef TT_JSON5_USE_WSTR
        result += std::to_wstring(lineNumber + 1);
#else
        result += std::to_string(lineNumber + 1);
#endif
        result += makeString(", column: ");
#ifdef TT_JSON5_USE_WSTR
        result += std::to_wstring(columnNumber - 1);
#else
        result += std::to_string(columnNumber - 1);
#endif
        result += makeString(". ");
        result += parseError;
        return result;
    }

    void Parser::parse(istream_t& stream, Value& result) {
        // Return null if file is empty.
        if (peek1(stream) == '\0') {
            result.type = ValueType::Null;
            return;
        }

#ifdef TT_JSON5_SUPPORT_MORE_WHITESPACE
        skipWhitespace(stream);
#endif

        parseValue(stream, result);
        if (errorCode != 0) return;
        if (stream.eof()) return;

#ifdef TT_JSON5_SUPPORT_MORE_WHITESPACE
        skipWhitespace(stream);
#endif

        char_t next = read1(stream);
        if (stream.eof()) {
            clearError();
            return;
        }

        throwParseError(makeString("Unexpected '") + next + makeString("' after value. Expected end of file."));
    }

    namespace {
        void indent(ostream_t& out, const char_t* tab, int depth) {
            if (!tab) return;
            for (int i = 0; i < depth; ++i)
                out << tab;
        }

        void newLine(ostream_t& out, const char_t* tab) {
            if (!tab) return;
            out << '\n';
        }
    }

    ifstream_t readUtf8(const std::string& path) {
        ifstream_t stream(path);
        stream.imbue(std::locale("C.UTF-8"));
        return stream;
    }

    ofstream_t writeUtf8(const std::string& path) {
        ofstream_t stream(path);
        stream.imbue(std::locale("C.UTF-8"));
        return stream;
    }

    void serialize(const Value& value, ostream_t& out, const char_t* tab, int depth) {
        switch (value.type) {
        case ValueType::Int:
            out << value.iValue;
            break;
        case ValueType::Double: {
            sstr_t Idontknowhowtodothisproperly;
            Idontknowhowtodothisproperly << value.dValue;
            str_t stringstreamshouldjustaddthefractionforme = Idontknowhowtodothisproperly.str();
            if (stringstreamshouldjustaddthefractionforme.find('.') == str_t::npos)
                out << stringstreamshouldjustaddthefractionforme.c_str() << makeString(".0");
            else
                out << value.dValue;
            break;
        }
        case ValueType::String:
            out << '"';
            out << value.sValue.c_str();
            out << '"';
            break;
        case ValueType::Array:
            out << '[';
            {
                bool mode = value.aValue.size() != 0 && value.aValue[0].type == ValueType::Object;
                if (mode)
                    newLine(out, tab);
                for (std::vector<Value>::const_iterator e = value.aValue.begin(); e != value.aValue.end(); ++e) {
                    if (mode)
                        indent(out, tab, depth + 1);
                    serialize(*e, out, tab, depth + 1);
                    if (std::next(e) != value.aValue.end())
                        out << ',';
                    if (mode)
                        newLine(out, tab);
                }
                if (mode)
                    indent(out, tab, depth);
            }
            out << ']';
            break;
        case ValueType::Object:
            out << '{';
            if (value.oValue.size() != 0)
                newLine(out, tab);
            for (auto e = value.oValue.begin(); e != value.oValue.end(); ++e) {
                indent(out, tab, depth + 1);
                out << '"';
                out << e->first;
                out << makeString("\": ");
                serialize(e->second, out, tab, depth + 1);
                if (std::next(e) != value.oValue.end())
                    out << makeString(", ");
                newLine(out, tab);
            }
            indent(out, tab, depth);
            out << '}';
            break;
        case ValueType::Bool:
            out << makeString(value.bValue ? "true" : "false");
            break;
        case ValueType::Null:
            out << makeString("null");
            break;
        }
    }
}
#endif

// Avoid leaking defines
#ifdef TT_JSON5_OBJECT_SUPPORT_IDENTIFIER_NAMES_KEYS
#undef TT_JSON5_OBJECT_SUPPORT_IDENTIFIER_NAMES_KEYS
#endif
#ifdef TT_JSON5_OBJECT_SUPPORT_TRAILING_COMMA
#undef TT_JSON5_OBJECT_SUPPORT_TRAILING_COMMA
#endif
#ifdef TT_JSON5_ARRAY_SUPPORT_TRAILING_COMMA
#undef TT_JSON5_ARRAY_SUPPORT_TRAILING_COMMA
#endif
#ifdef TT_JSON5_STRING_SUPPORT_SINGLE_QUOTES
#undef TT_JSON5_STRING_SUPPORT_SINGLE_QUOTES
#endif
#ifdef TT_JSON5_STRING_SUPPORT_ESCAPE_LINE_BREAKS
#undef TT_JSON5_STRING_SUPPORT_ESCAPE_LINE_BREAKS
#endif
#ifdef TT_JSON5_STRING_SUPPORT_HEX
#undef TT_JSON5_STRING_SUPPORT_HEX
#endif
#ifdef TT_JSON5_STRING_SUPPORT_CHARACTER_ESCAPES
#undef TT_JSON5_STRING_SUPPORT_CHARACTER_ESCAPES
#endif
#ifdef TT_JSON5_NUMBER_SUPPORT_HEX
#undef TT_JSON5_NUMBER_SUPPORT_HEX
#endif
#ifdef TT_JSON5_NUMBER_SUPPPORT_FREE_DECIMAL
#undef TT_JSON5_NUMBER_SUPPPORT_FREE_DECIMAL
#endif
#ifdef TT_JSON5_NUMBER_SUPPORT_INF_AND_NAN
#undef TT_JSON5_NUMBER_SUPPORT_INF_AND_NAN
#endif
#ifdef TT_JSON5_NUMBER_SUPPORT_PLUS_SIGN
#undef TT_JSON5_NUMBER_SUPPORT_PLUS_SIGN
#endif
#ifdef TT_JSON5_SUPPORT_SINGLE_LINE_COMMENTS 
#undef TT_JSON5_SUPPORT_SINGLE_LINE_COMMENTS 
#endif
#ifdef TT_JSON5_SUPPORT_BLOCK_COMMENTS 
#undef TT_JSON5_SUPPORT_BLOCK_COMMENTS 
#endif
#ifdef TT_JSON5_SUPPORT_MORE_WHITESPACE
#undef TT_JSON5_SUPPORT_MORE_WHITESPACE
#endif
