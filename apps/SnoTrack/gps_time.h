#pragma once

/** GPS time is usually measured in weeks and seconds, so these two functions convert to time_t.
 */
time_t GpsWeekNumberToUnixTime(int gps_week_number);
time_t GpsTimeToUnixTime(int gps_week_number, int time_of_week_ms);

/** The same, but for FILETIME. */
FILETIME GpsTimeToFileTime(int gps_week_number, int time_of_week_ms);
