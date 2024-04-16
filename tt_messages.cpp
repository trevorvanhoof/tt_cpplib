#include "tt_messages.h"
#include "windont.h"
#include <cstdio>
#include <corecrt_wstdio.h>

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

    wchar_t* _formatStr(const TT::ConstWStringView& fmt, va_list args) {
        size_t size;
#pragma warning(suppress:4996)    // 28719
        size = _vsnwprintf(nullptr, 0, fmt.start, args);

        wchar_t* message = new wchar_t[size + 1u];
        _vsnwprintf_s(message, size + 1u, size + 1u, fmt.start, args);
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
    char* formatStr(ConstStringView fmt, ...) {
        va_list args;
        __crt_va_start(args, fmt);
        char* message = _formatStr(fmt, args);
        __crt_va_end(args);
        return message;
    }

    // Caller owns the return value
    wchar_t* formatStr(ConstWStringView fmt, ...) {
        va_list args;
        __crt_va_start(args, fmt);
        wchar_t* message = _formatStr(fmt, args);
        __crt_va_end(args);
        return message;
    }

	void info(ConstStringView fmt, ...) {
		va_list args;
		__crt_va_start(args, fmt);
		_message("Info", MB_OK | MB_ICONINFORMATION, fmt, args);
		__crt_va_end(args);
	}

	void warning(ConstStringView fmt, ...) {
		va_list args;
		__crt_va_start(args, fmt);
		_message("Warning", MB_OK | MB_ICONWARNING, fmt, args);
		__crt_va_end(args);
		if (IsDebuggerPresent())
			DebugBreak();
	}

	void error(ConstStringView fmt, ...) {
		va_list args;
		__crt_va_start(args, fmt);
		_message("Error", MB_OK | MB_ICONEXCLAMATION, fmt, args);
		__crt_va_end(args);
		if (IsDebuggerPresent())
			DebugBreak();
	}

	void fatal(ConstStringView fmt, ...) {
		va_list args;
		__crt_va_start(args, fmt);
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

	bool assert(bool expression, ConstStringView fmt, ...) {
		if (expression)
			return true;
		va_list args;
		__crt_va_start(args, fmt);
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

	void assertFatal(bool expression, ConstStringView fmt, ...) {
		if (expression)
			return;
		va_list args;
		__crt_va_start(args, fmt);
		_message("Error", MB_OK | MB_ICONEXCLAMATION, fmt, args);
		__crt_va_end(args);
		if (IsDebuggerPresent())
			DebugBreak();
		else
			ExitProcess(0);
	}
}
