#pragma once

LONG GetRegistryTimeZoneInformation(const TCHAR *tzName, TIME_ZONE_INFORMATION *tzi);
CString FormatUtcDateTimeInTzSpecificLocalTime(const COleDateTime &dateTime, const TIME_ZONE_INFORMATION *tzi);
CString FormatUtcUnixTimeInTzSpecificLocalTime(time_t unix_timestamp, const TIME_ZONE_INFORMATION *tzi);
CString FormatUtcFileTimeInTzSpecificLocalTime(FILETIME ft, const TIME_ZONE_INFORMATION *tzi);
CString FormatFileTimeMs(FILETIME ft, const TIME_ZONE_INFORMATION *tzi);
CString FormatElapsedFileTimeMs(FILETIME ftStart, FILETIME ftEnd);
