#include "config.h"
#include "trace.h"
#include "utf8.h"

std::wstring ConvertMultiByteToWide(const std::string &m, UINT codePage)
{
	int required = MultiByteToWideChar(codePage, 0, m.c_str(), (int)m.size(), NULL, 0);
	WCHAR *buffer = new WCHAR[required];
	int result = MultiByteToWideChar(codePage, 0, m.c_str(), (int)m.size(), buffer, required);
	ASSERT(required == result);

	std::wstring str;
	str.assign(buffer, result);
	delete [] buffer;

	return str;
}

std::string ConvertWideToMultiByte(const std::wstring &w, UINT codePage)
{
	int required = WideCharToMultiByte(codePage, 0, w.c_str(), (int)w.size(), NULL, 0, NULL, NULL);
	char *buffer = new char[required];
	int result = WideCharToMultiByte(codePage, 0, w.c_str(), (int)w.size(), buffer, required, NULL, NULL);
	ASSERT(required == result);

	utf8_string str;
	str.assign(buffer, result);
	delete [] buffer;

	return str;
}

std::wstring ConvertNarrowToWide(const std::string &s)
{
	return ConvertMultiByteToWide(s, CP_ACP);
}

std::string ConvertWideToNarrow(const std::wstring &w)
{
	return ConvertWideToMultiByte(w, CP_ACP);
}

std::wstring ConvertUTF8ToWide(const utf8_string &u)
{
	return ConvertMultiByteToWide(u, CP_UTF8);
}

utf8_string ConvertWideToUTF8(const std::wstring &w)
{
	return ConvertWideToMultiByte(w, CP_UTF8);
}

#if defined(UNICODE)
std::wstring ConvertSystemToWide(const tstring &s)
{
	return s;
}
#else
std::wstring ConvertSystemToWide(const tstring &s)
{
	return ConvertNarrowToWide(s);
}
#endif

#if defined(UNICODE)
tstring ConvertWideToSystem(const std::wstring &w)
{
	return w;
}
#else
tstring ConvertWideToSystem(const std::wstring &w)
{
	return ConvertWideToNarrow(w);
}
#endif

#if defined(UNICODE)
tstring ConvertNarrowToSystem(const std::string &s)
{
	return ConvertNarrowToWide(s);
}
#else
tstring ConvertNarrowToSystem(const std::string &s)
{
	return s;
}
#endif

utf8_string ConvertSystemToUTF8(const tstring &s)
{
	// Unfortunately, if we're not Unicode, we need to round-trip via Wide:
	std::wstring w = ConvertSystemToWide(s);
	utf8_string u = ConvertWideToUTF8(w);

	// Then convert from Wide back to UTF8:
	return u;
}

tstring ConvertUTF8ToSystem(const utf8_string &u)
{
	// Again, if we're not Unicode, we need to round-trip via Wide:
	std::wstring w = ConvertUTF8ToWide(u);
	tstring t = ConvertWideToSystem(w);

	return t;
}

tstring ConvertASCIIToSystem(const std::string &a)
{
	// If we assume that we'll not use any characters > 127,
	// then it doesn't particularly matter what the user's codepage is set to,
	// and we can do this:
	return ConvertNarrowToSystem(a);
}

