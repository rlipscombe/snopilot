/* safe_snprintf.h
 *
 * Copyright (C) 2002-2003 Roger Lipscombe, http://www.differentpla.net/~roger/
 */

#ifndef SAFE_SNPRINTF_H
#define SAFE_SNPRINTF_H 1

#include <stdio.h>
#include <stdarg.h>
#include <tchar.h>

size_t safe_vsnprintf(char *buffer, size_t bufferLength, const char *fmt, va_list v);
size_t safe_vsnwprintf(wchar_t *buffer, size_t bufferLength, const wchar_t *fmt, va_list v);

/** snprintf doesn't truncate the buffer if the string doesn't fit.
 * This fixes that.
 */
size_t safe_snprintf(char *buffer, size_t bufferLength, const char *fmt, ...);
size_t safe_snwprintf(wchar_t *buffer, size_t bufferLength, const wchar_t *fmt, ...);

#if defined(_UNICODE)
 #define safe_vsntprintf	safe_vsnwprintf
 #define safe_sntprintf		safe_snwprintf
#else
 #define safe_vsntprintf	safe_vsnprintf
 #define safe_sntprintf		safe_snprintf
#endif

#endif /* SAFE_SNPRINTF_H */
