#pragma once

#include <vector>
#include <string>
#include <sstream>
#include <string_view>

namespace TT {
	size_t find(const std::string_view text, const std::string_view substr, size_t offset = 0);
	std::vector<size_t> findAll(const std::string_view text, const std::string_view substr);

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
	std::string formatStr(const std::string_view fmt, ...);
	std::wstring formatStr(const std::wstring_view fmt, ...);

	std::vector<std::string> split(const std::string_view s, const std::string_view delimiter);
}
