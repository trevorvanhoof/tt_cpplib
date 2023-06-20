#pragma once

#include "tt_strings.h"

namespace TT {
	void info(const ConstStringView& fmt, ...);
	void warning(const ConstStringView& fmt, ...);
	void error(const ConstStringView& fmt, ...);
	void fatal(const ConstStringView& fmt, ...);
#ifdef assert
#undef assert
#endif
	bool assert(bool expression);
	bool assert(bool expression, const ConstStringView& fmt, ...);
	void assertFatal(bool expression);
	void assertFatal(bool expression, const ConstStringView& fmt, ...);
}
