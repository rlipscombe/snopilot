#include "StdAfx.h"
#include "TimeZoneInformation.h"
#include "filetime_helpers.h"

#include <pshpack1.h>
struct REGISTRY_TZI
{
	LONG Bias;
	LONG StandardBias;
	LONG DaylightBias;
	SYSTEMTIME StandardDate;
	SYSTEMTIME DaylightDate;
};
#include <poppack.h>

LONG GetRegistryTimeZoneInformation(const TCHAR *tzName, TIME_ZONE_INFORMATION *tzi)
{
	CString tzKey = _T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Time Zones\\");
	tzKey += tzName;

	HKEY kTimeZone;
	LONG result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, tzKey, 0, KEY_READ, &kTimeZone);
	if (result != 0)
		return result;

	ASSERT(sizeof(REGISTRY_TZI) == 44);

	REGISTRY_TZI reg_tzi;
	DWORD cb = sizeof(REGISTRY_TZI);
	result = RegQueryValueEx(kTimeZone, _T("TZI"), NULL, NULL, (BYTE *)&reg_tzi, &cb);
	if (result == 0)
	{
		ASSERT(cb == sizeof(REGISTRY_TZI));
		tzi->Bias = reg_tzi.Bias;
		tzi->StandardBias = reg_tzi.StandardBias;
		tzi->DaylightBias = reg_tzi.DaylightBias;

		tzi->DaylightDate = reg_tzi.StandardDate;
		tzi->StandardDate = reg_tzi.DaylightDate;

		// Get the other information.
		DWORD cb = sizeof(tzi->StandardName)-1;
		RegQueryValueEx(kTimeZone, _T("Std"), NULL, NULL, (BYTE *)tzi->StandardName, &cb);
		cb = sizeof(tzi->DaylightName)-1;
		RegQueryValueEx(kTimeZone, _T("Dlt"), NULL, NULL, (BYTE *)tzi->DaylightName, &cb);
	}

	RegCloseKey(kTimeZone);
	return result;
}

CString FormatUtcSystemTimeInTzSpecificLocalTime(SYSTEMTIME utc, const TIME_ZONE_INFORMATION *tzi)
{
	// SystemTimeToTzSpecificLocalTime isn't const-correct, so make a copy.
	TIME_ZONE_INFORMATION temp = *tzi;

	SYSTEMTIME local;
	VERIFY(SystemTimeToTzSpecificLocalTime(&temp, &utc, &local));

	// Just because the timezone isn't the one the user's in doesn't mean that we want it formatted according
	// to that timezone's norms.
	CString strDate;
	GetDateFormat(LOCALE_USER_DEFAULT, DATE_LONGDATE, &local, NULL, strDate.GetBuffer(_MAX_PATH), _MAX_PATH);
	strDate.ReleaseBuffer();

	CString strTime;
	GetTimeFormat(LOCALE_USER_DEFAULT, 0, &local, NULL, strTime.GetBuffer(_MAX_PATH), _MAX_PATH);
	strTime.ReleaseBuffer();

	CString s;
	s.Format(_T("%s %s"), (LPCTSTR)strDate, (LPCTSTR)strTime);
	return s;
}

CString FormatUtcDateTimeInTzSpecificLocalTime(const COleDateTime &dateTime, const TIME_ZONE_INFORMATION *tzi)
{
	// Get the time in local time.
	SYSTEMTIME utc;
	dateTime.GetAsSystemTime(utc);

	return FormatUtcSystemTimeInTzSpecificLocalTime(utc, tzi);
}

CString FormatUtcUnixTimeInTzSpecificLocalTime(time_t unix_timestamp, const TIME_ZONE_INFORMATION *tzi)
{
	COleDateTime dateTime(unix_timestamp);

	return FormatUtcDateTimeInTzSpecificLocalTime(dateTime, tzi);
}

CString FormatUtcFileTimeInTzSpecificLocalTime(FILETIME ft, const TIME_ZONE_INFORMATION *tzi)
{
	SYSTEMTIME st;
	VERIFY(FileTimeToSystemTime(&ft, &st));

	return FormatUtcSystemTimeInTzSpecificLocalTime(st, tzi);
}

time_t GetUnixTimeFromSystemTime(SYSTEMTIME st)
{
	struct tm tm;
	tm.tm_year = st.wYear-1900;
	tm.tm_mon = st.wMonth-1;
	tm.tm_mday = st.wDay;
	tm.tm_hour = st.wHour;
	tm.tm_min = st.wMinute;
	tm.tm_sec = st.wSecond;

	time_t t = mktime(&tm);
	return t;
}

CString FormatElapsedFileTimeMs(FILETIME ftStart, FILETIME ftEnd)
{
	ULARGE_INTEGER uliStart = FileTimeToULargeInteger(ftStart);
	ULARGE_INTEGER uliEnd = FileTimeToULargeInteger(ftEnd);

	ULONGLONG deltaT = uliEnd.QuadPart - uliStart.QuadPart;

	ULONGLONG total_milliseconds = deltaT / FILETIME_TICKS_PER_MILLISECOND;
	ULONGLONG total_seconds = total_milliseconds / 1000;
	ULONGLONG total_minutes = total_seconds / 60;
	ULONGLONG total_hours = total_minutes / 60;

	DWORD milliseconds = (DWORD)(total_milliseconds % 1000);
	DWORD seconds = (DWORD)(total_seconds % 60);
	DWORD minutes = (DWORD)(total_minutes % 60);
	DWORD hours = (DWORD)(total_hours);

	CString s;
	s.Format("%d:%02d:%02d.%03d", hours, minutes, seconds, milliseconds);
	return s;
}

CString FormatFileTimeMs(FILETIME ft, const TIME_ZONE_INFORMATION *tzi)
{
	SYSTEMTIME utc;
	VERIFY(FileTimeToSystemTime(&ft, &utc));

	TIME_ZONE_INFORMATION temp(*tzi);

	SYSTEMTIME local;
	VERIFY(SystemTimeToTzSpecificLocalTime(&temp, &utc, &local));

	CString strDate;
	GetDateFormat(LOCALE_USER_DEFAULT, 0, &local, NULL, strDate.GetBuffer(_MAX_PATH), _MAX_PATH); 
	strDate.ReleaseBuffer();

	CString strTime;
	GetTimeFormat(LOCALE_USER_DEFAULT, TIME_FORCE24HOURFORMAT, &local, NULL, strTime.GetBuffer(_MAX_PATH), _MAX_PATH);
	strTime.ReleaseBuffer();

	DWORD milliseconds = ft.dwLowDateTime % FILETIME_TICKS_PER_SECOND;
	milliseconds /= FILETIME_TICKS_PER_MILLISECOND;

	CString strMilliseconds;
	strMilliseconds.Format("%03d", milliseconds);

	CString strDecimalSeparator;
	GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SDECIMAL, strDecimalSeparator.GetBuffer(6), 6);
	strDecimalSeparator.ReleaseBuffer();

	return strDate + _T(" ") + strTime + strDecimalSeparator + strMilliseconds;
}
