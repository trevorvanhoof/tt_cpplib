/*
Single header json parser.
TODO: Make json5 compliant.
TODO: Make cross-platform wrapper around WideCharToMultiByte.

---

Include like so in exactly 1 of your cpp files to generate the implementation:

#define TTJSON_IMPLEMENTATION
#include "tt_json.h"

Include regularly in other files.

In addition, define TTJSON_TRAILING_COMMA_SUPPORT to support trailing commas in list and object fields.

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
#include <sstream>
#include <algorithm>
#include <vector>
#include <unordered_map>
#include <iostream>

#define TTJSON_LONG_DOUBLE

// Avoid windows.h
extern "C" __declspec(dllimport) int __stdcall WideCharToMultiByte(
    unsigned int CodePage,
    unsigned long dwFlags,
    const wchar_t* lpWideCharStr,
    int cchWideChar,
    char* lpMultiByteStr,
    int cbMultiByte,
    const char* lpDefaultChar,
    int* lpUsedDefaultChar
    );

namespace TTJson {
	// List all natively supported json value types
	enum class ValueType {
		Int, Double, String, List, Object, Bool, Null
	};

#ifdef TTJSON_LONG_DOUBLE
	typedef long double scalar;
#else
	typedef double scalar;
#endif

	struct Value {
		ValueType type;

		bool bValue;
		long long iValue;
		scalar dValue;
		std::string sValue;
		std::vector<Value> lValue;
		std::unordered_map<std::string, Value> oValue;

		Value();
		Value(ValueType type);
		Value(bool value);
		Value(long long value);
		Value(std::string value);
		Value(const std::vector<Value>& value);
		Value(const std::unordered_map<std::string, Value>& value);
		Value(float value);
		Value(double value);
		Value(long double value);
	};

	class Parser2 {
		std::istream* data;
		char byte;
		size_t data_size;

		void ReadByte();
		void SkipWhitespace();
		bool AssertChr(char require);
		bool CharIsNumber(char chr);
		Value ConsumeNumber();
		std::string ConsumeString();
		Value ConsumeValue();
		std::vector<Value> ConsumeArray();
		Value ConsumeObject();

	public:
		std::string parseError;

		Parser2(std::istream& jsonData) : data(&jsonData) { }

		Value Parse();
	};

	class Parser {
		std::string text;
		size_t cursor = 0;

		void SkipWhitespace();
		bool AssertChr(char require);
		bool CharIsNumber(char chr);
		Value ConsumeNumber();
		std::string ConsumeString();
		Value ConsumeValue();
		std::vector<Value> ConsumeArray();
		Value ConsumeObject();

	public:
		std::string parseError;

		Parser(const std::string& jsonData) : text(jsonData) { }

		Value Parse();
	};

	void Serialize(const Value& value, std::ostream& out, const char* tab = nullptr, int depth = 0);
	
	#ifdef TTJSON_IMPLEMENTATION
	Value::Value() { type = ValueType::Null; }
	Value::Value(ValueType type) : type(type) {}
	Value::Value(bool value) { bValue = value; type = ValueType::Bool; }
	Value::Value(long long value) { iValue = value; type = ValueType::Int; }
	Value::Value(std::string value) { sValue = value; type = ValueType::String; }
	Value::Value(const std::vector<Value>& value) { lValue = value; type = ValueType::List; }
	Value::Value(const std::unordered_map<std::string, Value>& value) { oValue = value; type = ValueType::Object; }
	Value::Value(float value) { dValue = (scalar)value; type = ValueType::Double; }
	Value::Value(double value) { dValue = (scalar)value; type = ValueType::Double; }
	Value::Value(long double value) { dValue = (scalar)value; type = ValueType::Double; }

	void Parser2::ReadByte() {
		data->read(&byte, 1);
	}

	void Parser2::SkipWhitespace() {
		while (byte == ' ' || byte == '\t' || byte == '\r' || byte == '\n')
			ReadByte();
	}

	bool Parser2::AssertChr(char require) {
		if (byte != require) {
			std::stringstream tmp;
			tmp << "Parse error, expected '" << require << "', got '" << byte << "' at index " << data->tellg();
			parseError = tmp.str();
			return false;
		}
		return true;
	}

	bool Parser2::CharIsNumber(char chr) {
		return (chr >= '0' && chr <= '9' ||
			chr >= 'a' && chr <= 'A' ||
			chr >= 'e' && chr <= 'E' ||
			chr == '.' ||
			chr == '-');
	}

	Value Parser2::ConsumeNumber() {
		if (!parseError.empty()) return Value();
		
		std::string value;

		// Advance
		while (CharIsNumber(byte)) {
			value += byte; 
			ReadByte();
		}

		// Parse
		char* tmp;
		// Strictly speaking json numbers do not support:
		// - fractions without leading 0, like .1
		// - + sign prefixes
		// - hex and oct notation
		if (std::find(value.begin(), value.end(), 'e') != value.end() ||
			std::find(value.begin(), value.end(), 'E') != value.end() ||
			std::find(value.begin(), value.end(), '.') != value.end())
#ifdef TTJSON_LONG_DOUBLE
			return Value(_strtold_l(value.c_str(), &tmp, _locale_t()));
#else
			return Value(_strtod_l(value.c_str(), &tmp, _locale_t()));
#endif
		return Value(_strtoi64(value.c_str(), &tmp, 10));
	}

	std::string Parser2::ConsumeString() {
		if (!parseError.empty()) return "";

		if (!AssertChr('"')) return "";
		// Advance past the start
		ReadByte();
		std::string raw;

		// Track escape characters
		bool skip = false;
		while (!data->eof()) {
			raw += byte;
			ReadByte();
			if (skip) { skip = false; continue; }
			if (byte == '"') break;
			skip = byte == '\\';
			if (data->tellg() == data_size) {
				parseError = "Parse error: unexpected end of file while reading string.";
				return "";
			}
		}
		ReadByte();

		// TODO: use stringview?
		// Decode escaped character pairs
		size_t sentinel = raw.size();
		for (size_t i = 0; i < sentinel; ++i) {
			if (raw[i] == '\\') {
				if (raw[i + 1] == 'b')
					raw[i] = '\b';
				else if (raw[i + 1] == '\\')
					raw[i] = '\\';
				else if (raw[i + 1] == 'n')
					raw[i] = '\n';
				else if (raw[i + 1] == 't')
					raw[i] = '\t';
				else if (raw[i + 1] == 'r')
					raw[i] = '\r';
				else if (raw[i + 1] == '/')
					raw[i] = '/';
				else if (raw[i + 1] == 'f')
					raw[i] = 'f';
				else if (raw[i + 1] == '"')
					raw[i] = '"';
				else continue;
				raw.erase(i + 1, 1);
				sentinel--;
			}
		}
		// Decode unicode characters
		size_t start = 0;
		while (true) {
			start = raw.find("\\u", start);
			if (start == std::string::npos) break;
			char* hexStart = &raw[start + 2];
			char* hexEnd = hexStart + 4;
			unsigned short utf16 = (unsigned short)_strtoi64(hexStart, &hexEnd, 16);
			const wchar_t* data = (wchar_t*)&utf16;
			int canNotConvert = 0;
			int bufSize = WideCharToMultiByte(65001, 0, (wchar_t*)&utf16, 1, nullptr, 0, "?", &canNotConvert);
			if (bufSize) {
				char* tmp = new char[bufSize];
				WideCharToMultiByte(65001, 0, (wchar_t*)&utf16, 1, tmp, bufSize, "?", &canNotConvert);
				raw = raw.substr(0, start) + std::string(tmp, bufSize) + raw.substr(start + 6);
				start += bufSize;
				delete[] tmp;
			} else {
				raw = raw.substr(0, start) + raw.substr(start + 6);
			}
		}
		return raw;
	}

	std::vector<Value> Parser2::ConsumeArray() {
		if (!parseError.empty()) return {};

		if (!AssertChr('[')) return {};

		ReadByte();
		std::vector<Value> result;
		while (true) {
			SkipWhitespace();
			// Detect end of list
			if (byte == ']') {
				ReadByte();
				break;
			}
			if (result.size() > 0) {
				// Detect comma
				if (!AssertChr(',')) return result;
				ReadByte();
				SkipWhitespace();
			}
#ifdef TTJSON_TRAILING_COMMA_SUPPORT
			// Detect end of list again to support trailing commas
			if (byte == ']') {
				ReadByte();
				break;
			}
#endif
			// Read value and proceed
			result.push_back(ConsumeValue());

			if (!parseError.empty()) return result;
		}
		return result;
	}

	Value Parser2::ConsumeObject() {
		if (!parseError.empty()) return {};

		if (!AssertChr('{')) return Value();

		ReadByte();
		std::unordered_map<std::string, Value> result;
		while (true) {
			SkipWhitespace();
			// Detect end of object
			if (byte == '}') {
				ReadByte();
				break;
			}
			if (result.size() > 0) {
				// Detect comma
				if (!AssertChr(',')) return result;
				ReadByte();
				SkipWhitespace();
			}

#ifdef TTJSON_TRAILING_COMMA_SUPPORT
			// Detect end of object again to support trailing commas
			if (text[cursor] == '}') {
				cursor++;
				break;
			}
#endif

			// Read key
			std::string key = ConsumeString();

			if (!parseError.empty()) return result;

			// Detect separator
			SkipWhitespace();
			if (!AssertChr(':')) return result;
			ReadByte();
			SkipWhitespace();
			// Read value
			result[key] = ConsumeValue();

			if (!parseError.empty()) return result;
		}
		return Value(result);
	}

	Value Parser2::ConsumeValue() {
		if (!parseError.empty()) return {};

		if (byte == '"')
			return Value(ConsumeString());
		if (byte == '[')
			return Value(ConsumeArray());
		if (byte == '{')
			return Value(ConsumeObject());

		// Look-ahead for null and bool
		size_t cursor = data->tellg();
		if (cursor + 5 < data_size) {
			char buf[5];
			data->read(buf, 5);
			std::transform(buf, buf + 5, buf, [](unsigned char c) { return std::tolower(c); });
			if (strcmp(buf, "false") == 0) {
				cursor += 5;
				return Value(false);
			}
			data->seekg(-5, std::ios::cur);
		}
		if (cursor + 4 < data_size) {
			char buf[5];
			data->read(buf, 4);
			std::transform(buf, buf + 5, buf, [](unsigned char c) { return std::tolower(c); });
			if (strcmp(buf, "true") == 0) {
				cursor += 4;
				return Value(true);
			}
			if (strcmp(buf, "null") == 0) {
				cursor += 4;
				return Value();
			}
			data->seekg(-4, std::ios::cur);
		}

		size_t tmp = data->tellg();
		return ConsumeNumber();
	}

	Value Parser2::Parse() {
		// Reset the stream and read the first byte to work with
		data->seekg(0, std::ios::end);
		data_size = data->tellg();
		data->seekg(0, std::ios::beg); ReadByte(); 
		SkipWhitespace();
		return ConsumeValue();
	}

	void Parser::SkipWhitespace() {
		char* pCursor = &text[cursor];
		while (*pCursor == ' ' || *pCursor == '\t' || *pCursor == '\r' || *pCursor == '\n') {
			++pCursor;
		}
		cursor = pCursor - &text[0];
	}

	bool Parser::AssertChr(char require) {
		if (text[cursor] != require) {
			parseError = "Parse error, expected '{require}', got '{_text[_cursor]}' at index {cursor}.";
			return false;
		}
		return true;
	}

	bool Parser::CharIsNumber(char chr) {
		return (chr >= '0' && chr <= '9' ||
			chr >= 'a' && chr <= 'A' ||
			chr >= 'e' && chr <= 'E' ||
			chr == '.' ||
			chr == '-');
	}

	Value Parser::ConsumeNumber() {
		if (!parseError.empty()) return Value();

		size_t start = cursor;
		// Advance
		char* pCursor = &text[cursor];
		while (CharIsNumber(*pCursor)) pCursor++;
		cursor = pCursor - &text[0];
		// Parse
		std::string value = text.substr(start, cursor - start);
		// Strictly speaking json numbers do not support:
		// - fractions without leading 0, like .1
		// - + sign prefixes
		// - hex and oct notation
		if (std::find(value.begin(), value.end(), 'e') != value.end() ||
			std::find(value.begin(), value.end(), 'E') != value.end() ||
			std::find(value.begin(), value.end(), '.') != value.end())
#ifdef TTJSON_LONG_DOUBLE
			return Value(_strtold_l(&text[start], &pCursor, _locale_t()));
#else
			return Value(_strtod_l(&text[start], &pCursor, _locale_t()));
#endif
		return Value(_strtoi64(&text[start], &pCursor, 10));
	}

	std::string Parser::ConsumeString() {
		if (!parseError.empty()) return "";

		if (!AssertChr('"')) return "";
		// Advance past the start
		++cursor;
		size_t start = cursor;
		// Track escape characters
		bool skip = false;
		while (cursor < text.size()) {
			char chr = text[cursor++];
			if (skip) { skip = false; continue; }
			if (chr == '"') break;
			skip = chr == '\\';
			if (cursor == text.size()) {
				parseError = "Parse error: unexpected end of file while reading string.";
				return "";
			}
		}
		// TODO: use stringview?
		std::string raw = text.substr(start, cursor - start - 1);
		// Decode escaped character pairs
		size_t sentinel = raw.size();
		for (size_t i = 0; i < sentinel; ++i) {
			if (raw[i] == '\\') {
				if (raw[i + 1] == 'b')
					raw[i] = '\b';
				else if (raw[i + 1] == '\\')
					raw[i] = '\\';
				else if (raw[i + 1] == 'n')
					raw[i] = '\n';
				else if (raw[i + 1] == 't')
					raw[i] = '\t';
				else if (raw[i + 1] == 'r')
					raw[i] = '\r';
				else if (raw[i + 1] == '/')
					raw[i] = '/';
				else if (raw[i + 1] == 'f')
					raw[i] = 'f';
				else if (raw[i + 1] == '"')
					raw[i] = '"';
				else continue;
				raw.erase(i + 1, 1);
				sentinel--;
			}
		}
		// Decode unicode characters
		start = 0;
		while (true) {
			start = raw.find("\\u", start);
			if (start == std::string::npos) break;
			char* hexStart = &raw[start + 2];
			char* hexEnd = hexStart + 4;
			unsigned short utf16 = (unsigned short)_strtoi64(hexStart, &hexEnd, 16);
			const wchar_t* data = (wchar_t*)&utf16;
			int canNotConvert = 0;
			int bufSize = WideCharToMultiByte(65001, 0, (wchar_t*)&utf16, 1, nullptr, 0, "?", &canNotConvert);
			if (bufSize) {
				char* tmp = new char[bufSize];
				WideCharToMultiByte(65001, 0, (wchar_t*)&utf16, 1, tmp, bufSize, "?", &canNotConvert);
				raw = raw.substr(0, start) + std::string(tmp, bufSize) + raw.substr(start + 6);
				start += bufSize;
				delete[] tmp;
			} else {
				raw = raw.substr(0, start) + raw.substr(start + 6);
			}
		}
		return raw;
	}

	std::vector<Value> Parser::ConsumeArray() {
		if (!parseError.empty()) return {};

		if (!AssertChr('[')) return {};

		cursor++;
		std::vector<Value> result;
		while (true) {
			SkipWhitespace();
			// Detect end of list
			if (text[cursor] == ']') {
				cursor++;
				break;
			}
			if (result.size() > 0) {
				// Detect comma
				if (!AssertChr(',')) return result;
				cursor++;
				SkipWhitespace();
			}
			#ifdef TTJSON_TRAILING_COMMA_SUPPORT
			// Detect end of list again to support trailing commas
			if (text[cursor] == ']') {
				cursor++;
				break;
			}
			#endif
			// Read value and proceed
			result.push_back(ConsumeValue());

			if (!parseError.empty()) return result;
		}
		return result;
	}

	Value Parser::ConsumeObject() {
		if (!parseError.empty()) return {};

		if (!AssertChr('{')) return Value();

		cursor++;
		std::unordered_map<std::string, Value> result;
		while (true) {
			SkipWhitespace();
			// Detect end of object
			if (text[cursor] == '}') {
				cursor++;
				break;
			}
			if (result.size() > 0) {
				// Detect comma
				if (!AssertChr(',')) return result;
				cursor++;
				SkipWhitespace();
			}

			#ifdef TTJSON_TRAILING_COMMA_SUPPORT
			// Detect end of object again to support trailing commas
			if (text[cursor] == '}') {
				cursor++;
				break;
			}
			#endif

			// Read key
			std::string key = ConsumeString();

			if (!parseError.empty()) return result;

			// Detect separator
			SkipWhitespace();
			if (!AssertChr(':')) return result;
			cursor++;
			SkipWhitespace();
			// Read value
			result[key] = ConsumeValue();

			if (!parseError.empty()) return result;
		}
		return Value(result);
	}

	Value Parser::ConsumeValue() {
		if (!parseError.empty()) return {};

		char chr = text[cursor];
		if (chr == '"')
			return Value(ConsumeString());
		if (chr == '[')
			return Value(ConsumeArray());
		if (chr == '{')
			return Value(ConsumeObject());
		// Look-ahead for null and bool
		if (cursor + 5 < text.size()) {
			std::string tmp = text.substr(cursor, 5);
			std::transform(tmp.begin(), tmp.end(), tmp.begin(), [](unsigned char c) { return std::tolower(c); });
			if (tmp == "false") {
				cursor += 5;
				return Value(false);
			}
		} else if (cursor + 4 < text.size()) {
			std::string tmp = text.substr(cursor, 4);
			std::transform(tmp.begin(), tmp.end(), tmp.begin(), [](unsigned char c) { return std::tolower(c); });
			if (tmp == "true") {
				cursor += 4;
				return Value(true);
			}
			if (tmp == "null") {
				cursor += 4;
				return Value();
			}
		}
		return ConsumeNumber();
	}

	Value Parser::Parse() {
		cursor = 0;
		SkipWhitespace();
		return ConsumeValue();
	}

	namespace {
		void Indent(std::ostream& out, const char* tab, int depth) {
			if (!tab) return;
			for (int i = 0; i < depth; ++i)
				out << tab;
		}

		void NewLine(std::ostream& out, const char* tab) {
			if (!tab) return;
			out << '\n';
		}
	}

	void Serialize(const Value& value, std::ostream& out, const char* tab, int depth) {
		switch (value.type) {
		case ValueType::Int:
			out << value.iValue;
			break;
		case ValueType::Double:
			out << value.dValue;
			break;
		case ValueType::String:
			out << '"';
			out << value.sValue.c_str();
			out << '"';
			break;
		case ValueType::List:
			out << "[";
			{
				bool mode = value.lValue.size() != 0 && value.lValue[0].type == ValueType::Object;
				if(mode)
					NewLine(out, tab);
				for (std::vector<Value>::const_iterator e = value.lValue.begin(); e != value.lValue.end(); ++e) {
					if (mode)
						Indent(out, tab, depth + 1);
					Serialize(*e, out, tab, depth + 1);
					if (std::next(e) != value.lValue.end())
						out << ", ";
					if (mode) 
						NewLine(out, tab);
				}
				if (mode)
					Indent(out, tab, depth);
			}
			out << "]";
			break;
		case ValueType::Object:
			out << "{";
			if (value.oValue.size() != 0)
				NewLine(out, tab); 
			for (auto e = value.oValue.begin(); e != value.oValue.end(); ++e) {
				Indent(out, tab, depth + 1);
				out << '"';
				out << e->first;
				out << "\": ";
				Serialize(e->second, out, tab, depth + 1);
				if (std::next(e) != value.oValue.end())
					out << ", ";
				NewLine(out, tab);
			}
			Indent(out, tab, depth);
			out << "}";
			break;
		case ValueType::Bool:
			out << (value.bValue ? "true" : "false");
			break;
		case ValueType::Null:
			out << "null";
			break;
		}
	}
	#endif
}
