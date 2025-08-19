#ifndef LOGGER_H
#define LOGGER_H

/*
	%c	character.
	%d	decimal (integer) number (base 10).
	%e	exponential floating-point number.
	%f	floating-point number.
	%i	integer (base 10).
	%o	octal number (base 8).
	%s	a string of characters.
	%u	unsigned decimal (integer) number.
	%x	number in hexadecimal (base 16).
	%%	print a percent sign.
	\%	print a percent sign.
 */

#include <_windows.h>

#define LOG(format, ...) log_log(LOG_LEVEL_INFO, __FILE__, __LINE__, format, __VA_ARGS__)
#define LOG_WARNING(format, ...) log_log(LOG_LEVEL_WARN, __FILE__, __LINE__, format, __VA_ARGS__)
#define LOG_ERR(format, ...) log_log(LOG_LEVEL_ERROR, __FILE__, __LINE__, format, __VA_ARGS__)
#define LOG_FATAL(format, ...) log_log(LOG_LEVEL_FATAL, __FILE__, __LINE__, format, __VA_ARGS__)

typedef enum
{
	LOG_LEVEL_INFO,
	LOG_LEVEL_WARN,
	LOG_LEVEL_ERROR,
	LOG_LEVEL_FATAL
} LogLevel;

void log_log(LogLevel level, const char* file, int line, const char* fmt, ...);

#endif // LOGGER_H

// =====================================================================================
//                                  IMPLEMENTATION
// =====================================================================================

#ifdef LOGGER_IMPLEMENTATION

#define STB_SPRINTF_IMPLEMENTATION
#include "stb_sprintf.h"

static struct
{
	HANDLE hConsole;		   // Handle to the console output
	CRITICAL_SECTION crit_sec; // For thread-safe logging
	volatile LONG initialized; // Atomic flag for one-time initialization
} g_logger;

static constexpr WORD LOG_COLORS[] = {
	FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,										// LOG_LEVEL_INFO  -> White
	FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY,									// LOG_LEVEL_WARN  -> Bright Yellow
	FOREGROUND_RED | FOREGROUND_INTENSITY,														// LOG_LEVEL_ERROR -> Bright Red
	BACKGROUND_RED | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY // LOG_LEVEL_FATAL -> Bright White on Red BG
};

static const char* LOG_LEVEL_STRINGS[] = {
	"INFO ", "WARN ", "ERROR", "FATAL"
};

int fast_sprintf(char* dest, const char* fmt, ...) {
	va_list argptr;

	va_start(argptr, fmt);
	const int retValue = stbsp_vsprintf(dest, fmt, argptr);
	va_end(argptr);

	return retValue;
}

/**
 * @brief The core logging function that formats and prints messages.
 *
 * @param level The log level (e.g., LOG_LEVEL_INFO).
 * @param file The source file name where the log was called.
 * @param line The line number in the source file.
 * @param fmt The format string (printf-style).
 * @param ... Variable arguments for the format string.
 */
void log_log(LogLevel level, const char* file, int line, const char* fmt, ...)
{
	if (InterlockedCompareExchange(&g_logger.initialized, 1, 0) == 0)
	{
		g_logger.hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		InitializeCriticalSection(&g_logger.crit_sec);
	}

	char buffer[4096];
	char* current_pos = buffer;

	SYSTEMTIME st;
	GetLocalTime(&st);
	current_pos += fast_sprintf(current_pos, "%02d:%02d:%02d.%03d ", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

	const char* file_basename = file;
	for (const char* p = file; *p; ++p)
	{
		if (*p == '\\' || *p == '/')
		{
			file_basename = p + 1;
		}
	}

	current_pos += fast_sprintf(current_pos, "[%s] [%s:%d] ", LOG_LEVEL_STRINGS[level], file_basename, line);

	va_list args;
	va_start(args, fmt);
	current_pos += fast_sprintf(current_pos, fmt, args);
	va_end(args);

	*current_pos++ = '\n';

	EnterCriticalSection(&g_logger.crit_sec);

	SetConsoleTextAttribute(g_logger.hConsole, LOG_COLORS[level]);

	WriteConsoleA(g_logger.hConsole, buffer, (DWORD)(current_pos - buffer), NULL, NULL);

	SetConsoleTextAttribute(g_logger.hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);

	LeaveCriticalSection(&g_logger.crit_sec);

	if (level == LOG_LEVEL_FATAL)
	{
		ExitProcess(1);
	}
}

#endif // LOGGER_IMPLEMENTATION