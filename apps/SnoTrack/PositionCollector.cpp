#include "StdAfx.h"
#include "PositionCollector.h"
#include "tokenise_string.h"
#include "garmin/garmin_time.h"
#include "garmin/garmin_semicircles.h"
#include "win32/filetime_helpers.h"
#include "Projection.h"

bool TracklogPositionCollector::OnLine(const std::string &line)
{
	// TRK_IDENT,LAT,LON,TIME,ALT,DPTH,NEW,LAT_DEG,LON_DEG,TIME_GMT
	tokens_t tokens = TokeniseString(line, ',');
	if (tokens.size() < 10)
		return false;

	char *p, *q, *r, *s;
	std::string trk_ident = tokens[0];
	long lat_sem = strtol(tokens[1].c_str(), &p, 10);
	long lon_sem = strtol(tokens[2].c_str(), &q, 10);
	long garmin_timestamp = strtol(tokens[3].c_str(), &r, 10);
	double gps_elevation = strtod(tokens[4].c_str(), &s);
	char isNew = tokens[6][0];

	if (!*p && !*q && !*r && !*s)
	{
		double lon_deg = DegreesFromSemicircles(lon_sem);
		double lat_deg = DegreesFromSemicircles(lat_sem);
		FILETIME timestamp = FileTimeFromGarminTime(garmin_timestamp);

	    CPoint p = m_proj->ForwardDeg(lon_deg, lat_deg);
		PositionFix position(trk_ident.c_str(), lon_deg, lat_deg, timestamp, gps_elevation, (isNew == 'Y'), p.x, p.y);

		if (timestamp > m_highest_timestamp)
		{
			m_positions->push_back(position);
			m_highest_timestamp = timestamp;
		}
		else
		{
			TRACE("Warning! Retrograde position fix:\n");
//			TRACE("  this: %d, %f, %f, %f\n", position.GetUnixTimestamp(), position.GetLongitude(), position.GetLongitude(), position.GetGpsElevation());
		}
	}

	return true;
}

bool WaypointPositionCollector::OnLine(const std::string &line)
{
	tokens_t tokens = TokeniseString(line, ',');
	if (tokens.size() < 5)
		return false;

	char *p, *q;
	long lat_sem = strtol(tokens[0].c_str(), &p, 10);
	long lon_sem = strtol(tokens[1].c_str(), &q, 10);
	std::string label = tokens[4];

	if (!*p && !*q)
	{
		double lon_deg = DegreesFromSemicircles(lon_sem);
		double lat_deg = DegreesFromSemicircles(lat_sem);

	    CPoint p = m_proj->ForwardDeg(lon_deg, lat_deg);
		Waypoint waypoint(label.c_str(), lon_deg, lat_deg, p.x, p.y);
		m_waypoints->push_back(waypoint);
	}

	return true;
}
