#include "StdAfx.h"
#include "gps_time.h"
#include "unix_time.h"
#include "win32/filetime_helpers.h"

time_t GpsWeekNumberToUnixTime(int gps_week_number)
{
	// time_t counts the number of seconds since 1970/01/01, 00:00:00.  The GPS epoch is defined as follows:
	//   GPS Time is conventionally represented in GPS Weeks and GPS Seconds from the GPS Epoch,
	//   the transition from 23:59:59 UTC on Saturday January 5, 1980 to 00:00:00 UTC on
	//   Sunday, January 6th. This GPS week, January 6 through January 12, 1980 is
	//   week zero (0) and the week of August 15 though August 21, 1999 is week 1023.
	//   The week of August 22 through August 28, 1999 is week 1024.
	time_t gps_week_zero = MakeUnixTime(1980, 1, 6, 0, 0, 0);

	// There are 7 * 24 * 60 * 60 seconds in a week, so add those on.
	const int SECONDS_PER_WEEK = 7 * 24 * 60 * 60;
	return gps_week_zero + (gps_week_number * SECONDS_PER_WEEK);
}

time_t GpsTimeToUnixTime(int gps_week_number, int time_of_week_ms)
{
	return GpsWeekNumberToUnixTime(gps_week_number) + (time_of_week_ms / 1000);
}

FILETIME GpsTimeToFileTime(int gps_week_number, int time_of_week_ms)
{
	time_t unix_time = GpsTimeToUnixTime(gps_week_number, time_of_week_ms);
	FILETIME ft = FileTimeFromUnixTime(unix_time);

	int milliseconds = time_of_week_ms % 1000;
	ULARGE_INTEGER uli;
	uli.QuadPart = milliseconds * FILETIME_TICKS_PER_MILLISECOND;

	FILETIME add;
	add.dwHighDateTime = uli.HighPart;
	add.dwLowDateTime = uli.LowPart;

	return ft + add;
}
