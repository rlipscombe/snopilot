/* safe_snprintf.cpp
*
* Copyright (C) 2002-2003 Roger Lipscombe, http://www.differentpla.net/~roger/
*/

#include "config.h"
#include "trace.h"
#include "safe_snprintf.h"

size_t safe_vsnprintf(char *buffer, size_t bufferLength, const char *fmt, va_list v)
{
	int n = (int)_vsnprintf(buffer, bufferLength, fmt, v);
	if (n < 0 || n == (int)bufferLength)
		n = (int)bufferLength-1;

	// glibc 2.1 is a bit freaky -- it returns the number of characters that
	// would have been written.  Odd.
#if defined(__GLIBC__)
#if ((__GLIBC__ >= 2) && (__GLIBC_MINOR__ >= 1))
	if (n > (int)bufferLength)
		n = bufferLength-1;
#endif
#endif // __GLIBC__

	buffer[n] = 0;
	return n;
}

size_t safe_snprintf(char *buffer, size_t bufferLength, const char *fmt, ...)
{
	va_list v;

	va_start(v, fmt);
	size_t n = safe_vsnprintf(buffer, bufferLength, fmt, v);
	va_end(v);

	return n;
}

/** Unlike the C library's vsnprintf, this ensures that the buffer is correctly terminated.
 * It also supports wchar_t.
 * bufferLength is in characters, not bytes.
 */
size_t safe_vsnwprintf(wchar_t *buffer, size_t bufferLength, const wchar_t *fmt, va_list v)
{
	int n = (int)_vsnwprintf(buffer, bufferLength, fmt, v);
	if (n < 0 || n == (int)bufferLength)
		n = (int)bufferLength-1;

	// glibc 2.1 is a bit freaky -- it returns the number of characters that
	// would have been written.  Odd.
#if defined(__GLIBC__)
#if ((__GLIBC__ >= 2) && (__GLIBC_MINOR__ >= 1))
	if (n > (int)bufferLength)
		n = bufferLength-1;
#endif
#endif // __GLIBC__

	buffer[n] = 0;
	return n;
}

size_t safe_snwprintf(wchar_t *buffer, size_t bufferLength, const wchar_t *fmt, ...)
{
	va_list v;

	va_start(v, fmt);
	size_t n = safe_vsnwprintf(buffer, bufferLength, fmt, v);
	va_end(v);

	return n;
}

