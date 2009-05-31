/* trace.h
 *
 * Copyright (C) 2002-2004 Roger Lipscombe, http://www.differentpla.net/~roger/
 */

#ifndef TRACE_H
#define TRACE_H 1

#ifndef ENABLE_TRACE
 #define ENABLE_TRACE 1
#endif

#include <tchar.h>

void outputString(const _TCHAR *buffer);
void outputFormattedString(const _TCHAR *fmt, ...);

/** Important: Don't increase the size of this buffer without testing for overflows.
 * The particular problem is that, in Release builds, it'll allocate a chunk of the
 * stack past the guard page, and that this'll cause a fault when you try to access it.
 */
#define MAX_OUTPUT_BUFFER (1024)

#if defined(_MSC_VER)
 // Compatible with Visual Studio's Output window.
 #define OUTPUT_PREFIX_FMT _T("%hs(%d) : ")
#else
 // Compatible with gcc's compiler output.
 // Note that here we don't use %hs, because gcc will generate a warning (which can't be disabled),
 // and it'll just get tedious if there's a warning on _every_ call to TRACE.
 #define OUTPUT_PREFIX_FMT "%s:%d: "
#endif

class Tracer
{
	const char *m_file;
	int m_line;

public:
	Tracer(const char *file, int line)
		: m_file(file), m_line(line)
	{
	}

	void operator() (const char *fmt, ...);
};

/** @todo Come up with some template magic so that TRACEC(0,...) vanishes entirely. */
class ConditionalTracer
{
	const char *m_file;
	int m_line;

public:
	ConditionalTracer(const char *file, int line)
		: m_file(file), m_line(line)
	{
	}

	void operator() (bool output, const char *fmt, ...);
};

inline void DoNothing(...) { /* do nothing */ }

void TraceHex(const char *message, const BYTE *buffer, const BYTE *buffer_end, const char *file, int line);

// If we're compiling against MFC, we want to use our TRACE macro, not its.
#ifdef _MFC_VER
 #undef TRACE
#endif

#if ENABLE_TRACE
 #define TRACE Tracer(__FILE__, __LINE__)
 #define TRACEC ConditionalTracer(__FILE__, __LINE__)

 #define TRACE_HEX(m, b, e) \
	TraceHex((m), (const BYTE *)(b), (const BYTE *)(e), __FILE__, __LINE__)

#else
 #define TRACE DoNothing
 #define TRACEC DoNothing
#endif

#define TRACE_WARN TRACE

#if !defined(ASSERT)
 #include <assert.h>
 #define ASSERT assert
#endif

#if !defined(VERIFY)
 #if DEBUG>0
  #define VERIFY(x) ASSERT((x))
 #else
  #define VERIFY(x) (x)
 #endif
#endif

#endif /* TRACE_H */
