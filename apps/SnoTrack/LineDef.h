#pragma once

struct LineDef
{
	int m_x1, m_y1;
	int m_x2, m_y2;

	COLORREF m_cr;
	int m_width;

	FILETIME m_timestamp;
	ULONGLONG m_gap;

public:
	LineDef(int x1, int y1, int x2, int y2, COLORREF cr, int width, FILETIME timestamp, ULONGLONG gap)
		: m_x1(x1), m_y1(y1), m_x2(x2), m_y2(y2), m_cr(cr), m_width(width), m_timestamp(timestamp), m_gap(gap)
	{
	}

	LineDef(int x1, int y1, int x2, int y2, COLORREF cr, int width)
		: m_x1(x1), m_y1(y1), m_x2(x2), m_y2(y2), m_cr(cr), m_width(width), m_gap(0)
	{
		m_timestamp.dwHighDateTime = m_timestamp.dwLowDateTime = 0;
	}
};
