#pragma once

#include "line_collector.h"
#include "PositionFix.h"
#include "Waypoint.h"

class Projection;

class TracklogPositionCollector : public LineCollectorCallback
{
	FILETIME m_highest_timestamp;
	PositionFixCollection *m_positions;
	Projection *m_proj;

public:
	TracklogPositionCollector(PositionFixCollection *positions, Projection *proj)
		: m_positions(positions), m_proj(proj)
	{
		m_highest_timestamp.dwHighDateTime = 0;
		m_highest_timestamp.dwLowDateTime = 0;
	}

// LineCollectorCallback
public:
	virtual bool OnLine(const std::string &line);
};

/** @todo Almost the same -- can we merge them if we use a little more intelligence? */
class WaypointPositionCollector : public LineCollectorCallback
{
	WaypointCollection *m_waypoints;
	Projection *m_proj;

public:
	WaypointPositionCollector(WaypointCollection *waypoints, Projection *proj)
		: m_waypoints(waypoints), m_proj(proj)
	{
	}

// LineCollectorCallback
public:
	virtual bool OnLine(const std::string &line);
};
