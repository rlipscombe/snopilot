#pragma once
#include <string>
#include <vector>
#include "utf8.h"

class Waypoint
{
	double m_lonDeg;			///< Where was this fix taken?
	double m_latDeg;			///< Where was this fix taken?

	utf8_string m_label;		///< What it's called, if it has a name.

	int m_proj_x;			///< Once it's been projected, the x value goes here.
	int m_proj_y;			///< Once it's been projected, the y value goes here.

public:
	Waypoint(const utf8_string &label, double lon_deg, double lat_deg, int proj_x, int proj_y)
		: m_label(label), m_lonDeg(lon_deg), m_latDeg(lat_deg), m_proj_x(proj_x), m_proj_y(proj_y)
	{
	}

	utf8_string GetLabel() const { return m_label; }

	double GetLongitude() const { return m_lonDeg; }
	double GetLatitude() const { return m_latDeg; }

	int GetProjectionX() const { return m_proj_x; }
	int GetProjectionY() const { return m_proj_y; }
};
typedef std::vector< Waypoint > WaypointCollection;
