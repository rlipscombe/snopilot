#include "config.h"
#include "trace.h"
#include "unix_time.h"
#include <time.h>

/** A sane wrapper for mktime -- specifically, year is given as, e.g., 2004, month is one-based,
 * etc.
 * For example, 2004/05/24, 0:00:00 would be given as MakeUnixTime(2004, 5, 24, 0, 0, 0)
 */
time_t MakeUnixTime(int year, int month, int day, int hour, int minute, int second)
{
	// Make it easy to figure out which time_t we're supposed to be using.
	struct tm t;
	memset(&t, 0, sizeof(struct tm));
	t.tm_year = year - 1900;
	t.tm_mon = month - 1;
	t.tm_mday = day;
	t.tm_hour = hour;
	t.tm_min = minute;
	t.tm_sec = second;

	return mktime(&t);
}
