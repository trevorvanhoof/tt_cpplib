#include "tt_messages.h"
#include "windont.h"
#include <cstdio>
#include <corecrt_wstdio.h>

namespace {
    // Caller owns the return value
    char* _formatStr(const std::string_view fmt, va_list args, size_t& size) {
#pragma warning(suppress:28719)    // 28719
        size = vsnprintf(nullptr, 0, fmt.data(), args);

        char* message = new char[size + 1u];
        vsnprintf(message, size + 1u, fmt.data(), args);
        message[size] = '\0';

        return message;
    }

    wchar_t* _formatStr(const std::wstring_view fmt, va_list args, size_t& size) {
#pragma warning(suppress:4996)    // 28719
        size = _vsnwprintf(nullptr, 0, fmt.data(), args);

        wchar_t* message = new wchar_t[size + 1u];
        _vsnwprintf_s(message, size + 1u, size + 1u, fmt.data(), args);
        message[size] = '\0';

        return message;
    }

	void _message(const std::string_view title, unsigned int flags, const std::string_view fmt, va_list args) {
        size_t size;
		const char* message = _formatStr(fmt, args, size);
		if (IsDebuggerPresent()) {
			OutputDebugStringA(message);
			OutputDebugStringA("\n");
		}
		else
			MessageBoxA(0, message, title.data(), flags);
		delete message;
	}
}

namespace TT {
    std::string formatStr(const std::string_view fmt, ...) {
        size_t size;
        va_list args;
        __crt_va_start(args, fmt);
        char* message = _formatStr(fmt, args, size);
        __crt_va_end(args);
        std::string result(message, size);
        delete[] message;
        return result;
    }

    std::wstring formatStr(const std::wstring_view fmt, ...) {
        size_t size;
        va_list args;
        __crt_va_start(args, fmt);
        wchar_t* message = _formatStr(fmt, args, size);
        __crt_va_end(args);
        std::wstring result(message, size);
        delete[] message;
        return result;
    }

	void info(const std::string_view fmt, ...) {
		va_list args;
		__crt_va_start(args, fmt);
		_message("Info", MB_OK | MB_ICONINFORMATION, fmt, args);
		__crt_va_end(args);
	}

	void warning(const std::string_view fmt, ...) {
		va_list args;
		__crt_va_start(args, fmt);
		_message("Warning", MB_OK | MB_ICONWARNING, fmt, args);
		__crt_va_end(args);
		if (IsDebuggerPresent())
			DebugBreak();
	}

	void error(const std::string_view fmt, ...) {
		va_list args;
		__crt_va_start(args, fmt);
		_message("Error", MB_OK | MB_ICONEXCLAMATION, fmt, args);
		__crt_va_end(args);
		if (IsDebuggerPresent())
			DebugBreak();
	}

	void fatal(const std::string_view fmt, ...) {
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

	bool assert(bool expression, const std::string_view fmt, ...) {
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

	void assertFatal(bool expression, const std::string_view fmt, ...) {
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
