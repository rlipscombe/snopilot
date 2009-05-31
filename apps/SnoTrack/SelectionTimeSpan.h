#pragma once

struct SelectionTimeSpan
{
	FILETIME m_startTime;
	FILETIME m_endTime;

	SelectionTimeSpan(FILETIME startTime, FILETIME endTime)
		: m_startTime(startTime), m_endTime(endTime)
	{
	}
};

typedef std::vector< SelectionTimeSpan > SelectionTimeSpanCollection;

struct SelectionTimeSpanLess
{
	bool operator() (const SelectionTimeSpan &lhs, const SelectionTimeSpan &rhs) const
	{
		return CompareFileTime(&lhs.m_startTime, &rhs.m_startTime) < 0;
	}
};
