#pragma once

#include <vector>
#include <string>

namespace TT {
	// Caller owns the return value
	char* FormatStr(const char* fmt, ...);
	size_t Find(const std::string& text, const std::string& substr, size_t offset);
	std::vector<size_t> FindAll(const std::string& text, const std::string& substr);
}
