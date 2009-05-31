#pragma once
#include <vector>
#include "utf8.h"

class Lift
{
	utf8_string m_label;

	double m_bot_lon;
	double m_bot_lat;
	int m_bot_x;
	int m_bot_y;
	
	double m_top_lon;
    double m_top_lat;
	int m_top_x;
	int m_top_y;

public:
	Lift(const utf8_string &label, double bot_lon, double bot_lat, int bot_x, int bot_y, double top_lon, double top_lat, int top_x, int top_y)
		: m_label(label),
			m_bot_lon(bot_lon), m_bot_lat(bot_lat), m_bot_x(bot_x), m_bot_y(bot_y),
			m_top_lon(top_lon), m_top_lat(top_lat), m_top_x(top_x), m_top_y(top_y)
	{
	}

	utf8_string GetLabel() const { return m_label; }
	double GetBottomLongitude() const { return m_bot_lon; }
	double GetBottomLatitude() const { return m_bot_lat; }

	int GetBottomProjectionX() const { return m_bot_x; }
	int GetBottomProjectionY() const { return m_bot_y; }

	double GetTopLongitude() const { return m_top_lon; }
	double GetTopLatitude() const { return m_top_lat; }

	int GetTopProjectionX() const { return m_top_x; }
	int GetTopProjectionY() const { return m_top_y; }
};
typedef std::vector< Lift > LiftCollection;
