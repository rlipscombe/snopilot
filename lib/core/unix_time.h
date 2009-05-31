#pragma once

/** A sane wrapper for mktime -- specifically, year is given as, e.g., 2004, month is one-based,
 * etc.
 * For example, 2004/05/24, 0:00:00 would be given as MakeUnixTime(2004, 5, 24, 0, 0, 0)
 */
time_t MakeUnixTime(int year, int month, int day, int hour, int minute, int second);
