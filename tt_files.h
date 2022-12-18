#pragma once

#include <string>

namespace TT {
	std::string ReadAllBytes(const char* filename);
	std::string ReadWithIncludes(const char* filename);
}
