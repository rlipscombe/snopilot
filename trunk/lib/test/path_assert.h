/* path_assert.h
 *
 * Copyright (C) 2002-2003 Roger Lipscombe, http://www.differentpla.net/~roger/
 * Copyright (c) 2003 Snopilot, http://www.snopilot.com/
 */

#ifndef PATH_ASSERT_H
#define PATH_ASSERT_H 1

#include <string>

// File does (or does not) exist.
#define assertFileExists(filename) \
    (void)(PathFileExists(filename) || \
	test_assertFileExistsFailed(filename, __FILE__, __LINE__), 0)

int test_assertFileExistsFailed(const char *filename, const char *file, int line);

#define assertFileDoesNotExist(filename) \
    (void)((!PathFileExists(filename)) || \
	test_assertFileDoesNotExistFailed(filename, __FILE__, __LINE__), 0)

int test_assertFileDoesNotExistFailed(const char *filename, const char *file, int line);

// Path is (is not) a directory.
#define assertPathIsDirectory(pathname) \
    (void)(PathIsDirectory(pathname) || \
	test_assertPathIsDirectoryFailed(pathname, __FILE__, __LINE__), 0)

int test_assertPathIsDirectoryFailed(const char *pathname, const char *file, int line);

#define assertPathIsNotDirectory(pathname) \
    (void)((!PathIsDirectory(pathname)) || \
	test_assertPathIsNotDirectoryFailed(pathname, __FILE__, __LINE__), 0)

int test_assertPathIsNotDirectoryFailed(const char *pathname, const char *file, int line);

// Path is a subdirectory of another directory.
#define assertDirectoryExistsInPath(parent, child) \
	assertPathIsDirectory(AppendPath(parent, child).c_str())

// File does (does not) exist in a given directory.
#define assertFileExistsInDirectory(parent, child) \
	assertFileExists(AppendPath(parent, child).c_str())

#define assertFileDoesNotExistInDirectory(parent, child) \
	assertFileDoesNotExist(AppendPath(parent, child).c_str())

// Are two files identical?
#define assertFilesIdentical(a, b) \
	(void)(FilesIdentical(a, b) || \
	test_assertFilesIdenticalFailed(a, b, __FILE__, __LINE__), 0)

int test_assertFilesIdenticalFailed(const std::string &a, const std::string &b, const char *file, int line);

#endif /* PATH_ASSERT_H */
