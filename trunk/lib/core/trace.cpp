/* trace.cpp
 *
 * Copyright (C) 2002-2003 Roger Lipscombe, http://www.differentpla.net/~roger/
 */

#include "config.h"
#include "trace.h"
#include <tchar.h>
#include <stdlib.h>
#include "safe_snprintf.h"

#if defined(WIN32)
static void outputDebugString(const _TCHAR *buffer)
{
	OutputDebugString(buffer);
	OutputDebugString(_T("\r\n"));
}
#else
inline void outputDebugString(const char *buffer)
{
	UNUSED(buffer);
}
#endif

void outputString(const _TCHAR *buffer)
{
	_ftprintf(stderr, _T("%s\n"), buffer);
	fflush(stderr);

	outputDebugString(buffer);
}

void outputFormattedString(const _TCHAR *fmt, ...)
{
	_TCHAR buffer[MAX_OUTPUT_BUFFER];

	va_list v;

	va_start(v, fmt);

	safe_vsntprintf(buffer, COUNTOF(buffer), fmt, v);
	va_end(v);

	outputString(buffer);
}

/** Because the %s values in the arg list change width, we use safe_vsntprintf.
 * Unfortunately, because the fmt string is always narrow, and safe_vsntprintf
 * takes one that varies, we need (when _UNICODE is defined) to convert
 * fmt to a wide string, using mbstowcs.
 *
 * Another alternative is to always use safe_vsnprintf and to go through the
 * format string converting %s.  Because safe_vsnprintf assumes that %s is
 * always narrow, and (in Unicode builds) we want it to be wide, we need
 * to replace it with %ls (or more easily %S).  Similarly for %c.  Obviously,
 * if the format string already contains a %ls or %hs, we need to leave it alone.
 *
 * We can't use safe_vsnwprintf in the same way, because it requires that the
 * a wide format string, which would mean that the TRACE macro would have to
 * stick an 'L' on it.  Since it can't -- we're past that point when using
 * the constructor/operator() mechanism, this is not a viable option.
 */
static void outputTraceMessage(const char *file, int line, const char *fmt, va_list v)
{
	_TCHAR buffer[MAX_OUTPUT_BUFFER];
	size_t charsStored = safe_sntprintf(buffer, COUNTOF(buffer),
		OUTPUT_PREFIX_FMT, file, line);

	// We need to widen 'fmt':
#if defined _UNICODE
	wchar_t temp[512];
	size_t result = mbstowcs(temp, fmt, COUNTOF(temp));
	if (result == COUNTOF(temp))	// It wasn't successfully terminated, so we'll do it ourselves.
		temp[result-1] = L'\0';
#else
	const char *temp = fmt;
#endif

	// Because safe_snprintf returns the number of _characters_ stored
	// in the buffer, the following pointer addition is safe, as is
	// the subtraction.  It'll never be negative, because charsStored
	// is not allowed to be >= COUNTOF(buffer).
	size_t charsAppended = safe_vsntprintf(buffer + charsStored, COUNTOF(buffer) - charsStored, temp, v);

	// We're going to be outputting our own line-ending in a moment.  Make sure
	// that we don't do it twice.
	size_t charCount = charsStored + charsAppended;
	if (buffer[charCount-1] == '\n')
		buffer[charCount-1] = '\0';
	outputString(buffer);
}

void Tracer::operator() (const char *fmt, ...)
{
	va_list v;
	va_start(v, fmt);

	outputTraceMessage(m_file, m_line, fmt, v);
    va_end(v);
}

void ConditionalTracer::operator() (bool output, const char *fmt, ...)
{
	if (!output)
		return;

	va_list v;
	va_start(v, fmt);
	outputTraceMessage(m_file, m_line, fmt, v);
    va_end(v);
}

void TraceHex(const char *message, const BYTE *buffer, const BYTE *buffer_end, const char *file, int line)
{
	if (buffer == buffer_end)
	{
		outputFormattedString(OUTPUT_PREFIX_FMT _T("%hs <empty>"), file, line, message);
		return;
	}

	outputFormattedString(OUTPUT_PREFIX_FMT _T("%hs"), file, line, message);

	const int BYTES_PER_LINE = 16;

	const int HEX_BUFFER_WIDTH = (BYTES_PER_LINE * 3) + 1;
	const int CHR_BUFFER_WIDTH = BYTES_PER_LINE + 1;

	char hex_buffer[HEX_BUFFER_WIDTH];
	char chr_buffer[CHR_BUFFER_WIDTH];

	// First we'll dump all of the lines that are definitely full.
	const BYTE *p = buffer;
	for (; p < buffer_end - BYTES_PER_LINE; p += BYTES_PER_LINE)
	{
		for (int i = 0; i < BYTES_PER_LINE; ++i)
		{
			int hex_offset = (i * 3);
			int chr_offset = i;

			safe_snprintf(hex_buffer + hex_offset, HEX_BUFFER_WIDTH - hex_offset, "%02x ", p[i]);
			safe_snprintf(chr_buffer + chr_offset, CHR_BUFFER_WIDTH - chr_offset, "%c", isprint(p[i]) ? p[i] : '.');
		}

		outputFormattedString(_T("%hs  %hs"), hex_buffer, chr_buffer);
	}

	// There'll be a little bit left.
	int bytes_remaining = (int)(buffer_end - p);
	for (int i = 0; i < BYTES_PER_LINE; ++i)
	{
		int hex_offset = (i * 3);
		int chr_offset = i;

		if (i < bytes_remaining)
		{
			safe_snprintf(hex_buffer + hex_offset, HEX_BUFFER_WIDTH - hex_offset, "%02x ", p[i]);
			safe_snprintf(chr_buffer + chr_offset, CHR_BUFFER_WIDTH - chr_offset, "%c", isprint(p[i]) ? p[i] : '.');
		}
		else
		{
			strcpy(hex_buffer + hex_offset, "   ");
			strcpy(chr_buffer + chr_offset, " ");
		}
	}

	outputFormattedString(_T("%hs  %hs"), hex_buffer, chr_buffer);
}
