#pragma once
#include "line_collector.h"
#include "utf8.h"
#include "PositionFix.h"

struct SnologState
{
	int gps_week_number;
	int time_of_week_ms;
	int fix_type;
	int fix_flags;
	int longitude;
	int latitude;
};

class Projection;

class SnologSentenceCollector : public LineCollectorCallback
{
	utf8_string m_trk_ident;
	PositionFixCollection *m_positions;
	Projection *m_proj;

	SnologState m_state;
	int m_last_time_of_week;

	int m_max_speed_3d;

public:
	SnologSentenceCollector(const utf8_string &trk_ident, PositionFixCollection *positions, Projection *proj)
		: m_trk_ident(trk_ident), m_positions(positions), m_proj(proj), m_last_time_of_week(-1), m_max_speed_3d(0)
	{
	}

	~SnologSentenceCollector()
	{
		TRACE("m_max_speed_3d = %d cm/s\n");
	}

// LineCollectorCallback
public:
	virtual bool OnLine(const std::string &line);

private:
	void OnSolutionPacket(int time_of_week_ms, int gps_week_number, int fix_type, int fix_flags, int satellite_count);
	void OnVelocityPacket(int time_of_week_ms, int north_vel, int east_vel, int down_vel, int speed_3d, int speed_2d, int heading);
	void OnTemperaturePacket(int time_of_week_ms, int temperature);
	void OnPositionPacket(int time_of_week_ms, int longitude, int latitude, int height_above_ellipsoid, int height_above_msl, int horiz_accuracy, int vert_accuracy);
	void OnAltitudePacket(int time_of_week_ms, int altitude);
	void ProcessAccumulatedState();
};
