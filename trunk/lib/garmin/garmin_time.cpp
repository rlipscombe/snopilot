#include "config.h"
#include "trace.h"
#include "garmin_time.h"
#include <time.h>
#include "win32/filetime_helpers.h"

std::string FormatGarminTime(long garmin_time)
{
	time_t unix_time;
	struct tm *ptm;
	char temp[MAX_PATH];

	unix_time = UnixTimeFromGarminTime(garmin_time);
	ptm = gmtime(&unix_time);
	strftime(temp, MAX_PATH, "%b %d %Y %H:%M:%S", ptm);

	return temp;
}

long GarminTimeFromFileTime(FILETIME ft)
{
	SYSTEMTIME st;
	VERIFY(FileTimeToSystemTime(&ft, &st));
	return GarminTimeFromUnixTime(GetUnixTimeFromSystemTime(st));
}

FILETIME FileTimeFromGarminTime(long garmin_time)
{
	return FileTimeFromUnixTime(UnixTimeFromGarminTime(garmin_time));
}
