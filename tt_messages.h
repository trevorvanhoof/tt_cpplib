#pragma once

#include "tt_strings.h"

namespace TT {
	void Info(const char* fmt, ...);
	void Warning(const char* fmt, ...);
	void Error(const char* fmt, ...);
	void Fatal(char* fmt, ...);
	void Assert(bool expression);
	void Assert(bool expression, char* fmt, ...);
	void AssertFatal(bool expression);
	void AssertFatal(bool expression, const char* fmt, ...);
}
