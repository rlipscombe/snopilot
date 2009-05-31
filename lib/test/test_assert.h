/* test_assert.h
 *
 * Simple test framework for C++.  Output assert failures to the screen, and to the
 * Visual C++ debugger, in a format suitable for pressing F4.
 *
 * Copyright (C) 2002-2004 Roger Lipscombe, http://www.differentpla.net/
 * Copyright (c) 2003-2004 Snopilot, http://www.snopilot.com/
 */

#ifndef TEST_ASSERT_H
#define TEST_ASSERT_H 1

#include <string>
#include "tstring.h"

// General assert failure
int test_assertFailed(const char *msg, const char *file, int line);

// String comparison failures
int test_assertStringsEqualFailed(const char *msg, const std::string &expected, const std::string &actual,
				  const char *file, int line);
int test_assertStringsNotEqualFailed(const char *msg, const std::string &expected, const std::string &actual,
				     const char *file, int line);
int test_assertStringsEqualFailed(const char *msg, const std::wstring &expected, const std::wstring &actual,
				  const char *file, int line);
int test_assertStringsNotEqualFailed(const char *msg, const std::wstring &expected, const std::wstring &actual,
				     const char *file, int line);
#if defined(_MFC_VER)
inline int test_assertStringsEqualFailed(const char *msg, const CString &expected, const CString &actual,
				  const char *file, int line)
{
	return test_assertStringsEqualFailed(msg, tstring(expected), tstring(actual), file, line);
}

inline int test_assertStringsNotEqualFailed(const char *msg, const CString &expected, const CString &actual,
				     const char *file, int line)
{
	return test_assertStringsNotEqualFailed(msg, tstring(expected), tstring(actual), file, line);
}
#endif

// Integer comparison failures
int test_assertIntegersEqualFailed(const char *msg, int expected, int actual, const char *file, int line);
int test_assertIntegersNotEqualFailed(const char *msg, int expected, int actual, const char *file, int line);

int test_assertDoublesEqualFailed(const char *msg, double expected, double actual, const char *file, int line);
int test_assertFloatsEqualFailed(const char *msg, float expected, float actual, const char *file, int line);

int test_assertBooleansEqualFailed(int expected, int actual, const char *file, int line);

// Pointer comparison failures
int test_assertPointersEqualFailed(const char *msg, const void *expected, const void *actual, const char *file, int line);

// return assertSummarise at the end of main() -- it displays "All tests passed" where relevant.
int assertSummarise();

#define assertFailed(expr) \
    (void)((((signed long)expr)<0) || \
        test_assertFailed("assertFailed failed: " #expr, __FILE__, __LINE__), 0)

#define assertSucceeded(expr) \
    (void)((((signed long)expr)>=0) || \
        test_assertFailed("assertSucceeded failed: " #expr, __FILE__, __LINE__), 0)

#define assertTrue(expr) \
    (void)((expr) || \
	test_assertFailed("assertTrue failed: " #expr, __FILE__, __LINE__), 0)

#define assertFalse(expr) \
    (void)(!(expr) || \
	test_assertFailed("assertFalse failed: " #expr, __FILE__, __LINE__), 0)

#define assertStringsEqual(expected, actual) \
    (void)((stringsCompareEqual((expected), (actual))) || \
	test_assertStringsEqualFailed("assertStringsEqual failed: ", expected, actual, __FILE__, __LINE__), 0)

/**  Pointer comparison is *not* the correct way to compare strings.  Use a helper
 * function that converts to std::string instead.
 */
inline bool stringsCompareEqual(const std::string &lhs, const std::string &rhs)
{
	return (lhs == rhs);
}

inline bool stringsCompareEqual(const std::wstring &lhs, const std::wstring &rhs)
{
	return (lhs == rhs);
}

#if defined(_MFC_VER)
inline bool stringsCompareEqual(const CString &lhs, const CString &rhs)
{
	return (lhs == rhs);
}
#endif

/** To allow for precision errors, we treat the numbers as equal if rhs falls between (lhs - EPSILON) and (lhs + EPSILON)
 */
bool doublesCompareEqual(double expected, double actual, double epsilon);
bool floatsCompareEqual(float expected, float actual, float epsilon);

#define assertStringsNotEqual(expected, actual) \
    (void)((!stringsCompareEqual((expected), (actual))) || \
	test_assertStringsNotEqualFailed("assertStringsNotEqual failed: ", expected, actual, __FILE__, __LINE__), 0)

#define assertIntegersEqual(expected, actual) \
    (void)(((expected) == (actual)) || \
	test_assertIntegersEqualFailed("assertIntegersEqual failed: ", expected, actual, __FILE__, __LINE__), 0)

#define assertIntegersNotEqual(expected, actual) \
    (void)(((expected) != (actual)) || \
	test_assertIntegersNotEqualFailed("assertIntegersNotEqual failed: ", expected, actual, __FILE__, __LINE__), 0)

#define assertDoublesEqual(expected, actual, epsilon) \
	(void)((doublesCompareEqual((expected), (actual), (epsilon))) || \
	test_assertDoublesEqualFailed("assertDoublesEqual failed: ", expected, actual, __FILE__, __LINE__), 0)

#define assertFloatsEqual(expected, actual, epsilon) \
	(void)((floatsCompareEqual((expected), (actual), (epsilon))) || \
	test_assertFloatsEqualFailed("assertFloatsEqual failed: ", expected, actual, __FILE__, __LINE__), 0)

#define assertBooleansEqual(expected, actual) \
    (void)(((expected) == (actual)) || \
	test_assertBooleansEqualFailed(expected, actual, __FILE__, __LINE__), 0)

#define assertPointersEqual(expected, actual) \
	(void)(((expected) == (actual)) || \
	test_assertPointersEqualFailed("assertPointersEqual failed: ", expected, actual, __FILE__, __LINE__), 0)

// Inline helpers for std::string comparison.
inline int assertStringsEqualFailed(const std::string &expected, const std::string &actual,
				    const char *file, int line)
    { return assertStringsEqualFailed(expected.c_str(), actual.c_str(), file, line); }

void testOutputFailure();
void testAssertions();

#endif /* TEST_ASSERT_H */
