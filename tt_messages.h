#pragma once

#include <cstdio>

namespace TT {
	// Caller owns the return value
	char* FormatStr(const char* fmt, ...);
	void Info(const char* fmt, ...);
	void Warning(const char* fmt, ...);
	void Error(const char* fmt, ...);
	void Fatal(char* fmt, ...);
	void Assert(bool expression);
	void Assert(bool expression, char* fmt, ...);
	void AssertFatal(bool expression);
	void AssertFatal(bool expression, const char* fmt, ...);
}
