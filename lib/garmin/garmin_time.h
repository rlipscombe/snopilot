#pragma once
#include <string>

inline time_t UnixTimeFromGarminTime(long garmin_time)
{
	return garmin_time + 631065600;
}

inline long GarminTimeFromUnixTime(time_t unix_time)
{
	return (long)unix_time - 631065600;
}

long GarminTimeFromFileTime(FILETIME ft);
FILETIME FileTimeFromGarminTime(long garmin_time);

std::string FormatGarminTime(long garmin_time);
