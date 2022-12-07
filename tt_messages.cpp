#include "tt_messages.h"
#include <Windows.h>
#include <cstdio>

namespace TT {
	// Caller owns the return value
	static char* _FormatStr(const char* fmt, va_list args)
	{
		size_t size;
		#pragma warning(suppress:28719)    // 28719
		size = vsnprintf(nullptr, 0, fmt, args);

		char* message = new char[size + 1u];
		vsnprintf(message, size + 1u, fmt, args);
		message[size] = '\0';

		return message;
	}

	// Caller owns the return value
	char* FormatStr(const char* fmt, ...)
	{
		va_list args;
		__crt_va_start(args, fmt);
		char* message = _FormatStr(fmt, args);
		__crt_va_end(args);
		return message;
	}

	static void _Message(const char* title, unsigned int flags, const char* fmt, va_list args)
	{
		const char* message = _FormatStr(fmt, args);
		if (IsDebuggerPresent())
			OutputDebugStringA(message);
		else
			MessageBoxA(0, message, title, flags);
		delete message;
	}

	void Info(const char* fmt, ...)
	{
		va_list args;
		__crt_va_start(args, fmt);
		_Message("Info", MB_OK | MB_ICONINFORMATION, fmt, args);
		__crt_va_end(args);
	}

	void Warning(const char* fmt, ...)
	{
		va_list args;
		__crt_va_start(args, fmt);
		_Message("Warning", MB_OK | MB_ICONWARNING, fmt, args);
		__crt_va_end(args);
		if (IsDebuggerPresent())
			DebugBreak();
	}

	void Error(const char* fmt, ...)
	{
		va_list args;
		__crt_va_start(args, fmt);
		_Message("Error", MB_OK | MB_ICONEXCLAMATION, fmt, args);
		__crt_va_end(args);
		if (IsDebuggerPresent())
			DebugBreak();
	}

	void Fatal(char* fmt, ...)
	{
		va_list args;
		__crt_va_start(args, fmt);
		_Message("Error", MB_OK | MB_ICONEXCLAMATION, fmt, args);
		__crt_va_end(args);
		if (IsDebuggerPresent())
			DebugBreak();
		else
			ExitProcess(0);
	}

	void Assert(bool expression)
	{
		if (expression)
			return;
		if (IsDebuggerPresent())
			DebugBreak();
	}

	void Assert(bool expression, char* fmt, ...)
	{
		if (expression)
			return;
		va_list args;
		__crt_va_start(args, fmt);
		_Message("Error", MB_OK | MB_ICONEXCLAMATION, fmt, args);
		__crt_va_end(args);
		if (IsDebuggerPresent())
			DebugBreak();
	}

	void AssertFatal(bool expression)
	{
		if (expression)
			return;
		if (IsDebuggerPresent())
			DebugBreak();
		else
			ExitProcess(0);
	}

	void AssertFatal(bool expression, const char* fmt, ...)
	{
		if (expression)
			return;
		va_list args;
		__crt_va_start(args, fmt);
		_Message("Error", MB_OK | MB_ICONEXCLAMATION, fmt, args);
		__crt_va_end(args);
		if (IsDebuggerPresent())
			DebugBreak();
		else
			ExitProcess(0);
	}
}
