/* path_assert.cpp
 *
 * Simple test framework for C++.  Output assert failures to the screen, and to
 * the Visual C++ debugger, in a format suitable for pressing F4.
 *
 * Copyright (C) 2003 Roger Lipscombe, http://www.differentpla.net/~roger/
 * Copyright (c) 2003 Snopilot, http://www.snopilot.com/
 */

#include "config.h"
#include "trace.h"
#include "path_assert.h"

int outputFailure(const char *file, int line, const char *fmt, ...);

int test_assertFileExistsFailed(const char *filename, const char *file, int line)
{
	return outputFailure(file, line, "assertFileExists failed: '%s'", filename);
}

int test_assertFileDoesNotExistFailed(const char *filename, const char *file, int line)
{
	return outputFailure(file, line, "assertFileDoesNotExist failed: '%s'", filename);
}

int test_assertPathIsDirectoryFailed(const char *pathname, const char *file, int line)
{
	return outputFailure(file, line, "assertPathIsDirectory failed: '%s'", pathname);
}

int test_assertPathIsNotDirectoryFailed(const char *pathname, const char *file, int line)
{
	return outputFailure(file, line, "assertPathIsNotDirectory failed: '%s'", pathname);
}

int test_assertFilesIdenticalFailed(const std::string &a, const std::string &b, const char *file, int line)
{
	return outputFailure(file, line, "assertFilesIdentical failed: '%s' differs from '%s'", a.c_str(), b.c_str());
}
