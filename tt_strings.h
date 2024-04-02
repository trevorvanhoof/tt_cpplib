#pragma once

#include <vector>
#include <string>
#include <sstream>

namespace TT {
	struct StringView {
		char* start;
		const size_t count;

		StringView(std::string& text) : start(text.data()), count(text.size()) {}
		StringView(char* start) : start(start), count(strlen(start)) {}
		StringView(char* start, size_t count) : start(start), count(count) {}
	};

	struct ConstStringView {
		const char* start;
		const size_t count;

		ConstStringView(const std::string& text) : start(text.data()), count(text.size()) {}
		ConstStringView(const char* start) : start(start), count(strlen(start)) {}
		ConstStringView(const char* start, size_t count) : start(start), count(count) {}
	};

	size_t find(const ConstStringView& text, const ConstStringView& substr, size_t offset = 0);
	std::vector<size_t> findAll(const ConstStringView& text, const ConstStringView& substr);

	template<typename T>
	std::string join(std::vector<T> const& elements, const char* const delimiter) {
		std::ostringstream os;
		auto b = begin(elements), e = end(elements);

		if (b != e) {
			std::copy(b, prev(e), std::ostream_iterator<T>(os, delimiter));
			b = prev(e);
		}
		if (b != e) {
			os << *b;
		}

		return os.str();
	}

	// Caller owns the return value
	// Implemented in messages.cpp for static function reasons
	char* formatStr(const ConstStringView& fmt, ...);

	std::vector<std::string> split(std::string s, std::string delimiter);
}
