/* load_nmea.cpp
 *
 * Load a previously dumped NMEA trace, and parse it sensibly.
 *
 * Copyright (c) 2003 Snopilot, http://www.snopilot.com/
 */

#include "config.h"
#include "trace.h"
#include <stdio.h>
#include "line_collector.h"
#include <string>

/** Just dumps the line to the screen. */
class DebugLineCollectorCallback : public LineCollectorCallback
{
public:
	virtual bool OnLine(const std::string &line)
	{
		printf(">%s<\n", line.c_str());
		return true;
	}
};

/** The lines are prefixed with timestamps.  Holds the line back until it should come out. */
class TimestampedLines : public LineCollectorCallback
{
	/** In order to make this work, we need to know:
	 * a) How long it's supposed to be between the last line and the current line.
	 * b) How long it's been since we emitted the last line.
	 */
	FILETIME m_prev_emit_ft;	///< When did we output the last line?
	FILETIME m_prev_line_ft;	///< When was the last line captured?

public:
	TimestampedLines()
	{
		m_prev_emit_ft.dwHighDateTime = 0;
		m_prev_emit_ft.dwLowDateTime = 0;

		m_prev_line_ft.dwHighDateTime = 0;
		m_prev_line_ft.dwLowDateTime = 0;
	}

	/** Lines look like this:
	 * @ 2003/11/04 12:18:07.962: $PGRMZ,165,f,2*18
	 *           1         2         3
	 * 0123456789012345678901234567890
	 */
	virtual bool OnLine(const std::string &line)
	{
		// 1. Strip the timestamp from the beginning of the line.
		if (line[0] != '@')
		{
			fprintf(stderr, "Malformed line: '%s'\n", line.c_str());
			return false;
		}

		// The timestamp is fixed-width.
		std::string timestamp(line.substr(2, 23));
		std::string sentence(line.substr(27));

		SYSTEMTIME this_line_st;
		if (!TimestampToSystemTime(timestamp, &this_line_st))
		{
			fprintf(stderr, "Malformed timestamp: '%s'\n", timestamp.c_str());
			return false;
		}

		FILETIME this_line_ft;
		if (!SystemTimeToFileTime(&this_line_st, &this_line_ft))
		{
			fprintf(stderr, "Couldn't convert SYSTEMTIME to FILETIME.\n");		
			return false;
		}

		// 2. Work out how long since the previous timestamp it was.
		FILETIME this_emit_ft;
		for (;;)
		{
			SYSTEMTIME this_emit_st;
			GetSystemTime(&this_emit_st);
			SystemTimeToFileTime(&this_emit_st, &this_emit_ft);

			ULARGE_INTEGER this_emit_ul;
			this_emit_ul.HighPart = this_emit_ft.dwHighDateTime;
			this_emit_ul.LowPart = this_emit_ft.dwLowDateTime;

			ULARGE_INTEGER prev_emit_ul;
			prev_emit_ul.HighPart = m_prev_emit_ft.dwHighDateTime;
			prev_emit_ul.LowPart = m_prev_emit_ft.dwLowDateTime;

			ULARGE_INTEGER this_line_ul;
			this_line_ul.HighPart = this_line_ft.dwHighDateTime;
			this_line_ul.LowPart = this_line_ft.dwLowDateTime;

			ULARGE_INTEGER prev_line_ul;
			prev_line_ul.HighPart = m_prev_line_ft.dwHighDateTime;
			prev_line_ul.LowPart = m_prev_line_ft.dwLowDateTime;

			INT64 emit_delay = this_emit_ul.QuadPart - prev_emit_ul.QuadPart;
			INT64 line_delay = this_line_ul.QuadPart - prev_line_ul.QuadPart;

//			printf("%I64d / %I64d\n", emit_delay, line_delay);
			if (emit_delay >= line_delay)
			{
				// 3. If it's overdue, let it through.
				break;
			}
			else
			{
				// 4. If it's not, sleep for a bit.
				Sleep(10);
			}
		}

		printf(">%s< / >%s<\n", timestamp.c_str(), sentence.c_str());

		m_prev_line_ft = this_line_ft;
		m_prev_emit_ft = this_emit_ft;

		return true;
	}

public:
	bool TimestampToSystemTime(const std::string &timestamp, SYSTEMTIME *st) const
	{
		/* Timestamp looks like this:
		 * 2003/11/04 12:18:07.962
		 */

		std::string year(timestamp.substr(0, 4));
		std::string month(timestamp.substr(5, 2));
		std::string day(timestamp.substr(8, 2));

		std::string hour(timestamp.substr(11, 2));
		std::string minute(timestamp.substr(14, 2));
		std::string second(timestamp.substr(17, 2));
		std::string millisecond(timestamp.substr(20, 3));

//		printf("%s, %s, %s, %s, %s, %s, %s\n", year.c_str(), month.c_str(), day.c_str(), hour.c_str(), minute.c_str(), second.c_str(), millisecond.c_str());

		memset(st, 0, sizeof(SYSTEMTIME));
		st->wYear = atoi(year.c_str());
		st->wMonth = atoi(month.c_str());
		st->wDay = atoi(day.c_str());
		st->wHour = atoi(hour.c_str());
		st->wMinute = atoi(minute.c_str());
		st->wSecond = atoi(second.c_str());
		st->wMilliseconds = atoi(millisecond.c_str());

		return true;
	}
};

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		fprintf(stderr, "Usage:\n\tload_nmea filename\n");
		return 1;
	}

	TimestampedLines cb;
//	DebugLineCollectorCallback cb;
	LineCollector lineCollector(&cb);
	BufferCollector bufferCollector(&lineCollector);

	HANDLE hFile = CreateFile(argv[1], GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		for (;;)
		{
			char buffer[1025];
			int buffer_len = 1024;	// Allow space for the zero terminator.
			DWORD bytesRead;
			
			BOOL result = ReadFile(hFile, buffer, buffer_len, &bytesRead, NULL);
			if (!result)
			{
				fprintf(stderr, "ReadFile failed, error = %d.\n", GetLastError());
				return 1;
			}

			if (bytesRead != 0)
			{
				buffer[bytesRead] = '\0';

				bufferCollector.PushData(buffer, bytesRead);
			}
			else
				break;
		}

		CloseHandle(hFile);
	}

	return 0;
}
