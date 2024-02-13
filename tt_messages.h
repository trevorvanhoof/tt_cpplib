#pragma once

#include "tt_strings.h"

namespace TT {
	void info(ConstStringView fmt, ...);
	void warning(ConstStringView fmt, ...);
	void error(ConstStringView fmt, ...);
	void fatal(ConstStringView fmt, ...);
#ifdef assert
#undef assert
#endif
	bool assert(bool expression);
	bool assert(bool expression, ConstStringView fmt, ...);
	void assertFatal(bool expression);
	void assertFatal(bool expression, ConstStringView fmt, ...);
}
