/* config.h
 *
 * Copyright (C) 2002-2004 Roger Lipscombe, http://www.differentpla.net/
 * Copyright (C) 2003-2004 Snopilot, http://www.snopilot.com/
 */

#ifndef CONFIG_H
#define CONFIG_H 1

#if defined(WIN32)
// Might as well get it out of the way.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// POSIX/VC workarounds
#include <io.h>
#define open _open
#define read _read
#endif	/* WIN32 */

// Memory leak detection
#if defined(_MSC_VER)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <malloc.h>
#include <crtdbg.h>
#endif /* _MSC_VER */

#if defined(__GNUC__)
#define _vsnprintf vsnprintf
#endif /* __GNUC__ */

// MFC defines a few things for us already.
#if !defined(_MFC_VER)
 #include <assert.h>
 #define ASSERT assert

 #define UNUSED(x) (void)(x)
#endif /* _MFC_VER */

// Some useful macros
#define COUNTOF(a) ((sizeof((a))) / (sizeof((a)[0])))

#define DISALLOW_COPYING(X) \
	private: \
	X(const X&); \
	const X &operator= (const X&);

// Disabling warnings/Turning warnings into errors.
#if defined(_MSC_VER)
#pragma warning(disable:4786)	// 'identifier' : identifier was truncated to 'number' characters in the debug information
#pragma warning(error: 4715)	// 'function' : not all control paths return a value
#endif

// Error definitions
#ifndef FAILED
#define FAILED(expr) ((expr) < 0)
#endif

#ifndef SUCCEEDED
#define SUCCEEDED(expr) ((expr) >= 0)
#endif

#define NEW new

#endif /* CONFIG_H */
