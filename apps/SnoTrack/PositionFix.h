#pragma once
#include <vector>
#include "utf8.h"
#include "win32/filetime_helpers.h"

class PositionFix
{
	utf8_string m_trackIdent;
	FILETIME m_timeStamp;		///< When was this fix taken?

	double m_lonDeg;			///< Where was this fix taken?
	double m_latDeg;			///< Where was this fix taken?

	double m_gpsElevation;	///< Elevation, as returned by the GPS module.

	bool m_isNew;

	int m_proj_x;			///< Once it's been projected, the x value goes here.
	int m_proj_y;			///< Once it's been projected, the y value goes here.

public:
	enum Style
	{
		STYLE_FADED = -1,
		STYLE_NORMAL = 0,
		STYLE_HIGHLIGHT = 1,
	};

private:
	Style m_style;
	bool m_isHidden;

public:
	PositionFix()
		: m_trackIdent("NULL"), m_lonDeg(0.0), m_latDeg(0.0),
			m_gpsElevation(0.0), m_isNew(false), m_proj_x(0), m_proj_y(0), m_style(STYLE_NORMAL), m_isHidden(false)
	{
		m_timeStamp.dwHighDateTime = 0;
		m_timeStamp.dwLowDateTime = 0;
	}

	PositionFix(const utf8_string &trk_ident, double lon_deg, double lat_deg, FILETIME timeStamp, double gps_elevation, bool isNew, int proj_x, int proj_y)
		: m_trackIdent(trk_ident), m_lonDeg(lon_deg), m_latDeg(lat_deg), m_timeStamp(timeStamp),
			m_gpsElevation(gps_elevation), m_isNew(isNew), m_proj_x(proj_x), m_proj_y(proj_y), m_style(STYLE_NORMAL), m_isHidden(false)
	{
	}

	std::string GetTrackIdent() const { return m_trackIdent; }
	FILETIME GetTimestamp() const { return m_timeStamp; }

	double GetGpsElevation() const { return m_gpsElevation; }

	double GetLongitude() const { return m_lonDeg; }
	double GetLatitude() const { return m_latDeg; }

	bool IsNewTrack() const { return m_isNew; }

	int GetProjectionX() const { return m_proj_x; }
	int GetProjectionY() const { return m_proj_y; }

	void SetStyle(Style s) { m_style = s; }
	Style GetStyle() const { return m_style; }

	void SetHidden(bool b) { m_isHidden = b; }
	bool IsHidden() const { return m_isHidden; }

	int CompareTimestamp(const FILETIME &other) const { return FastCompareFileTime(&m_timeStamp, &other); }
	int CompareTimestamp(const PositionFix &other) const { return FastCompareFileTime(&m_timeStamp, &other.m_timeStamp); }
};
typedef std::vector< PositionFix > PositionFixCollection;

struct PositionsLessByTime
{
	bool operator() (const PositionFix &lhs, const PositionFix &rhs) const
	{
		return (lhs.CompareTimestamp(rhs) < 0);
	}

	bool operator() (const PositionFix &lhs, FILETIME rhs) const
	{
		return (lhs.CompareTimestamp(rhs) < 0);
	}

	bool operator() (FILETIME lhs, const PositionFix &rhs) const
	{
		return (rhs.CompareTimestamp(lhs) > 0);
	}
};

