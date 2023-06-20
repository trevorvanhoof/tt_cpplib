#include "tt_messages.h"
#include "windont.h"
#include <cstdio>

namespace {
	// Caller owns the return value
	char* _formatStr(const TT::ConstStringView& fmt, va_list args) {
		size_t size;
#pragma warning(suppress:28719)    // 28719
		size = vsnprintf(nullptr, 0, fmt.start, args);

		char* message = new char[size + 1u];
		vsnprintf(message, size + 1u, fmt.start, args);
		message[size] = '\0';

		return message;
	}

	void _message(const TT::ConstStringView& title, unsigned int flags, const TT::ConstStringView& fmt, va_list args) {
		const char* message = _formatStr(fmt, args);
		if (IsDebuggerPresent()) {
			OutputDebugStringA(message);
			OutputDebugStringA("\n");
		}
		else
			MessageBoxA(0, message, title.start, flags);
		delete message;
	}
}

namespace TT {
	// Caller owns the return value
	char* formatStr(const ConstStringView& fmt, ...) {
		va_list args;
		__crt_va_start(args, fmt.start);
		char* message = _formatStr(fmt, args);
		__crt_va_end(args);
		return message;
	}

	void info(const ConstStringView& fmt, ...) {
		va_list args;
		__crt_va_start(args, fmt.start);
		_message("Info", MB_OK | MB_ICONINFORMATION, fmt, args);
		__crt_va_end(args);
	}

	void warning(const ConstStringView& fmt, ...) {
		va_list args;
		__crt_va_start(args, fmt.start);
		_message("Warning", MB_OK | MB_ICONWARNING, fmt, args);
		__crt_va_end(args);
		if (IsDebuggerPresent())
			DebugBreak();
	}

	void error(const ConstStringView& fmt, ...) {
		va_list args;
		__crt_va_start(args, fmt.start);
		_message("Error", MB_OK | MB_ICONEXCLAMATION, fmt, args);
		__crt_va_end(args);
		if (IsDebuggerPresent())
			DebugBreak();
	}

	void fatal(const ConstStringView& fmt, ...) {
		va_list args;
		__crt_va_start(args, fmt.start);
		_message("Error", MB_OK | MB_ICONEXCLAMATION, fmt, args);
		__crt_va_end(args);
		if (IsDebuggerPresent())
			DebugBreak();
		else
			ExitProcess(0);
	}

#ifdef assert
#undef assert
#endif

	bool assert(bool expression) {
		if (expression)
			return true;
		if (IsDebuggerPresent())
			DebugBreak();
		return false;
	}

	bool assert(bool expression, const ConstStringView& fmt, ...) {
		if (expression)
			return true;
		va_list args;
		__crt_va_start(args, fmt.start);
		_message("Error", MB_OK | MB_ICONEXCLAMATION, fmt, args);
		__crt_va_end(args);
		if (IsDebuggerPresent())
			DebugBreak();
		return false;
	}

	void assertFatal(bool expression) {
		if (expression)
			return;
		if (IsDebuggerPresent())
			DebugBreak();
		else
			ExitProcess(0);
	}

	void assertFatal(bool expression, const ConstStringView& fmt, ...) {
		if (expression)
			return;
		va_list args;
		__crt_va_start(args, fmt.start);
		_message("Error", MB_OK | MB_ICONEXCLAMATION, fmt, args);
		__crt_va_end(args);
		if (IsDebuggerPresent())
			DebugBreak();
		else
			ExitProcess(0);
	}
}
