#pragma once

#include "tt_strings.h"

namespace TT {
	void info(const std::string_view fmt, ...);
	void warning(const std::string_view fmt, ...);
	void error(const std::string_view fmt, ...);
	void fatal(const std::string_view fmt, ...);
#ifdef assert
#undef assert
#endif
	bool assert(bool expression);
	bool assert(bool expression, const std::string_view fmt, ...);
	void assertFatal(bool expression);
	void assertFatal(bool expression, const std::string_view fmt, ...);
}
