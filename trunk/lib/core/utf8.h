#pragma once
#include <string>
#include "tstring.h"

/** Some definitions:
 * UTF8 = UTF8
 * Wide = UTF16.
 * Narrow = the user's current code page.
 * System = Whatever (Wide/Narrow) the system is currently using, as defined by the UNICODE macro.
 *
 * The PathName variants are there because there are some valid NTFS pathnames that don't survive
 * round-tripping via UTF8.  I don't remember the real issues, so the functions don't do
 * anything special.  They exist to encourage them to be used, so that they can be implemented properly
 * later.
 */

typedef char utf8_char;
typedef std::basic_string< utf8_char > utf8_string;

utf8_string ConvertSystemToUTF8(const tstring &s);

inline utf8_string ConvertPathNameToUTF8(const tstring &pathName)
{
	return ConvertSystemToUTF8(pathName);
}

tstring ConvertUTF8ToSystem(const utf8_string &u);

inline tstring ConvertUTF8ToPathName(const utf8_string &pathName)
{
	return ConvertUTF8ToSystem(pathName);
}

tstring ConvertASCIIToSystem(const std::string &a);

#if defined(_MFC_VER)
inline utf8_string ConvertSystemToUTF8(const CString &s)
{
	return ConvertSystemToUTF8(tstring(s));
}

inline utf8_string ConvertPathNameToUTF8(const CString &s)
{
	return ConvertPathNameToUTF8(tstring(s));
}
#endif