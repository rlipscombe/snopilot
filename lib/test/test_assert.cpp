/* test_assert.cpp
 *
 * Simple test framework for C++.  Output assert failures to the screen, and to
 * the Visual C++ debugger, in a format suitable for pressing F4.
 *
 * Copyright (C) 2002-2003 Roger Lipscombe, http://www.differentpla.net/~roger/
 * Copyright (c) 2003 Snopilot, http://www.snopilot.com/
 */

#include "config.h"
#include "trace.h"
#include "test_assert.h"
#include "safe_snprintf.h"
#include <stdio.h>
#include <vector>
#include <algorithm>
#include <float.h>	// for DBL_EPSILON

#if defined(WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#define TEST_ASSERT_FAILURES 0

static int assertFailures = 0;
typedef std::vector< std::string > failures_t;
static failures_t failures;

int outputFailure(const char *file, int line, const char *fmt, ...)
{
	char buffer[MAX_OUTPUT_BUFFER];
	size_t charsStored = safe_snprintf(buffer, COUNTOF(buffer),
		OUTPUT_PREFIX_FMT, file, line);

	va_list v;

	// Because safe_snprintf returns the number of _characters_ stored
	// in the buffer, the following pointer addition is safe, as is
	// the subtraction.  It'll never be negative, because charsStored
	// is not allowed to be >= COUNTOF(buffer).

	va_start(v, fmt);
	safe_vsnprintf(buffer + charsStored, COUNTOF(buffer) - charsStored, fmt, v);
	va_end(v);

	outputString(buffer);
	++assertFailures;
	failures.push_back(buffer);

	return 0;
}

static void outputFormattedString(const char *fmt, ...)
{
	char buffer[MAX_OUTPUT_BUFFER];

	va_list v;

	va_start(v, fmt);
	safe_vsnprintf(buffer, COUNTOF(buffer), fmt, v);
	va_end(v);

	outputString(buffer);
}

int test_assertFailed(const char *msg, const char *file, int line)
{
	return outputFailure(file, line, "%s", msg);
}

int test_assertStringsEqualFailed(const char *msg, const std::string &expected, const std::string &actual,
								  const char *file, int line)
{
	return outputFailure(file, line, "%s: expected: '%s', actual: '%s'",
		msg, expected.c_str(), actual.c_str());
}

int test_assertStringsNotEqualFailed(const char *msg, const std::string &expected, const std::string &actual,
									 const char *file, int line)
{
	return outputFailure(file, line, "%s: not expected: '%s', actual: '%s'",
		msg, expected.c_str(), actual.c_str());
}

int test_assertIntegersEqualFailed(const char *msg, int expected, int actual, const char *file, int line)
{
	return outputFailure(file, line, "%s: expected: %d (0x%x), actual: %d (0x%x)", msg, expected, expected, actual, actual);
}

int test_assertIntegersNotEqualFailed(const char *msg, int expected, int actual, const char *file, int line)
{
	return outputFailure(file, line, "%s: not expected: %d (0x%x), actual: %d (0x%x)", msg, expected, expected, actual, actual);
}

int test_assertDoublesEqualFailed(const char *msg, double expected, double actual, const char *file, int line)
{
	return outputFailure(file, line, "%s: expected: %e, actual: %e", msg, expected, actual);
}

int test_assertFloatsEqualFailed(const char *msg, float expected, float actual, const char *file, int line)
{
	return outputFailure(file, line, "%s: expected: %e, actual: %e", msg, expected, actual);
}

int test_assertBooleansEqualFailed(int expected, int actual, const char *file, int line)
{
	return outputFailure(file, line, "expected: %s, actual: %s",
		expected ? "true" : "false",
		actual ? "true" : "false");
}

int test_assertPointersEqualFailed(const char *msg, const void *expected, const void *actual, const char *file, int line)
{
	return outputFailure(file, line, "%s: expected: %p, actual: %p", msg, expected, actual);
}

void OutputStringFunctor(const std::string &s)
{
	outputString(s.c_str());
}

int assertSummarise()
{
	if (assertFailures == 0)
	{
		outputString("All tests passed.");
		return 0;
	}
	else
	{
		outputFormattedString("%d assert(s) triggered.  Tests failed:", assertFailures);
		std::for_each(failures.begin(), failures.end(), OutputStringFunctor);
		return 1;
	}
}

void testOutputFailure()
{
#if TEST_ASSERT_FAILURES
	char filename[MAX_PATH];
	memset(filename, 'a', MAX_PATH);
	outputFailure(filename, 42, "%s %d", "string", 0);
#endif
}

void testAssertions()
{
	// (0a) Some brief exercises for the assertion framework:
	{
		assertTrue(true);
		assertFalse(false);
		assertTrue(true || false);
		assertFalse(true && false);

		assertStringsEqual("this", "this");
		assertStringsNotEqual("this", "that");

		const char *p = "this";
		const char *q = "thisthis";
		q += 4;
		assertStringsEqual(p, q);

		std::string pp(p);
		std::string qq(q);
		assertStringsEqual(pp, qq);
	}

	// (0b) Some brief failure exercises for the assertion framework.
	// These tests are supposed to fail.  Set TEST_ASSERT_FAILURES to 1
	// if you want to check that you've not broken the assertions.
	{
#if TEST_ASSERT_FAILURES
		assertFalse(true);
		assertTrue(false);
		assertFalse(true || false);
		assertTrue(true && false);

		assertStringsNotEqual("this", "this");
		assertStringsEqual("this", "that");

		const char *p = "this";
		const char *q = "thisthis";
		q += 4;
		assertStringsNotEqual(p, q);

		std::string pp(p);
		std::string qq(q);
		assertStringsNotEqual(pp, qq);
#endif // TEST_ASSERT_FAILURES
	}
}

bool floatsCompareEqual(float expected, float actual, float epsilon)
{
	float upper = expected + epsilon;
	float lower = expected - epsilon;

	if (actual >= lower && actual <= upper)
		return true;

	return false;
}

bool doublesCompareEqual(double expected, double actual, double epsilon)
{
	double upper = expected + epsilon;
	double lower = expected - epsilon;

	if (actual >= lower && actual <= upper)
		return true;

	return false;
}
