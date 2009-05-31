#include "StdAfx.h"
#include "SnoLogCollector.h"
#include "tokenise_string.h"
#include "gps_time.h"
#include "Projection.h"

bool SnologSentenceCollector::OnLine(const std::string &line)
{
	tokens_t tokens = TokeniseString(line, ',');
	if (tokens.size() >= 1)
	{
		if (tokens[0] == "0")
		{
			// 0,comment
			TRACE("Comment: '%s'\n", tokens[1].c_str());
		}
		else if (tokens[0] == "1")
		{
			// NAV-POSLLH
			// 1,hex8-millisecond-time-of-week,hex8-longditude,
			// hex8-latitude,hex8-heightaboveellipsoid,hex8-heightabovemsl,
			// hex8-horizontalaccuracy,hex8-veriticalaccuracy
			// long/lat scaled 1e-7, heights and accuracies in mm
			//			TRACE("Position Solution\n");

			if (tokens.size() >= 8)
			{
				union { unsigned int u; int i; } t;

				// Time of week will never go signed as there aren't enough milliseconds
				// each week for that
				int time_of_week_ms = strtol(tokens[1].c_str(), NULL, 16);

				// Long/Lat are SIGNED, hence we need to strtoul() them then convert to
				// signed.
				t.u = strtoul(tokens[2].c_str(), NULL, 16);
				int longitude = t.i;
				t.u = strtoul(tokens[3].c_str(), NULL, 16);
				int latitude = t.i;

				// Heights are signed, though it's unlikely we'll see negative ones
				t.u = strtoul(tokens[4].c_str(), NULL, 16);
				int height_above_ellipsoid = t.i;
				t.u = strtoul(tokens[5].c_str(), NULL, 16);
				int height_above_msl = t.i;

				// Accuracies are always positive
				int horiz_accuracy = strtol(tokens[6].c_str(), NULL, 16);
				int vert_accuracy = strtol(tokens[7].c_str(), NULL, 16);

				OnPositionPacket(time_of_week_ms, longitude, latitude, height_above_ellipsoid, height_above_msl, horiz_accuracy, vert_accuracy);
			}
		}
		else if (tokens[0] == "2")
		{
			// NAV-SOL
			// 2,hex8-millisecondtimeofweek,hex4-weeknumber,
			// hex2-fixtype,hex2-fixflags,hex2-numberofsats
			//
			// weeknumber = gps week number
			// fixtype = 0,1 = no fix, 2=2d fix, 3=3d fix
			// fixflags =  0x08 time valid,
			//             0x04 week number valid
			// numberofsats = number of satellites used for fix
			if (tokens.size() >= 6)
			{
				int time_of_week_ms = strtol(tokens[1].c_str(), NULL, 16);
				int gps_week_number = strtol(tokens[2].c_str(), NULL, 16);
				int fix_type = strtol(tokens[3].c_str(), NULL, 16);
				int fix_flags = strtol(tokens[4].c_str(), NULL, 16);
				int satellite_count = strtol(tokens[5].c_str(), NULL, 16);

				OnSolutionPacket(time_of_week_ms, gps_week_number, fix_type, fix_flags, satellite_count);
			}
		}
		else if (tokens[0] == "3")
		{
			// NAV-VELNED
			// 3,hex8-millisecondtimeofweek,hex8-northvelocity,
			// hex8-eastvelocity,hex8-downvelocity,hex8-3dspeed,
			// hex8-2dspeed,hex8-heading,hex8-speedaccuracy
			//
			// velocities in cm/s
			// heading scaling 1e-5
			// accuracy in cm/s
			if (tokens.size() >= 9)
			{
				union { unsigned int u; int i; } t;

				int time_of_week_ms = strtol(tokens[1].c_str(), NULL, 16);

				// Velocities are signed
				t.u = strtoul(tokens[2].c_str(), NULL, 16);
				int north_vel = t.i;
				t.u = strtol(tokens[3].c_str(), NULL, 16);
				int east_vel =  t.i;
				t.u = strtol(tokens[4].c_str(), NULL, 16);
				int down_vel = t.i ;

				// Speed is unsigned
				int speed_3d = strtol(tokens[5].c_str(), NULL, 16);
				int speed_2d = strtol(tokens[6].c_str(), NULL, 16);
				int heading = strtol(tokens[7].c_str(), NULL, 16);
				int speed_accuracy = strtol(tokens[8].c_str(), NULL, 16);

				OnVelocityPacket(time_of_week_ms, north_vel, east_vel, down_vel, speed_3d, speed_2d, heading);
			}
		}
		else if (tokens[0] == "4")
		{
			// 4,hex8-millisecondtimeofweek,hex2-temperature
			// temperature in 2's complement, 0.5 degree resolution
			if (tokens.size() >= 3)
			{
				int time_of_week_ms = strtol(tokens[1].c_str(), NULL, 16);
				int temperature = strtol(tokens[2].c_str(), NULL, 16);

				OnTemperaturePacket(time_of_week_ms, temperature);
			}
		}
		else if (tokens[0] == "5")
		{
			// 5,hex8-millisecondtimeofweek,hex2-altitude
			// altitude sensor reading (raw)
			if (tokens.size() >= 3)
			{
				int time_of_week_ms = strtol(tokens[1].c_str(), NULL, 16);
				int altitude = strtol(tokens[2].c_str(), NULL, 16);

				OnAltitudePacket(time_of_week_ms, altitude);
			}
		}
	}

	return true;
}

void SnologSentenceCollector::OnSolutionPacket(int time_of_week_ms, int gps_week_number, int fix_type, int fix_flags, int satellite_count)
{
	if (time_of_week_ms != m_last_time_of_week)
	{
		// Then it's a new bunch of packets, process the accumulated state.
		ProcessAccumulatedState();
		m_last_time_of_week = time_of_week_ms;
	}

	//		TRACE("time_of_week_ms = %d ms\n", time_of_week_ms);
	//		TRACE("gps_week_number = %d\n", gps_week_number);
	//		TRACE("fix_type = %d (%s)\n", fix_type,
	//			(fix_type == 0) ? "No fix" :
	//			(fix_type == 1) ? "No fix" :
	//			(fix_type == 2) ? "2D fix" :
	//			(fix_type == 3) ? "3D fix" : "Other");
	//		TRACE("fix_flags = %d (%s %s)\n", fix_flags,
	//			(fix_flags & 0x08) ? "time valid" : "time invalid",
	//			(fix_flags & 0x04) ? "week valid" : "week invalid",
	//			(fix_flags & 0x02) ? "dgps valid" : "dgps invalid",
	//			(fix_flags & 0x01) ? "fix valid" : "fix invalid");
	//		TRACE("satellite_count = %d\n", satellite_count);

	m_state.gps_week_number = gps_week_number;
	m_state.time_of_week_ms = time_of_week_ms;
	m_state.fix_flags = fix_flags;
	m_state.fix_type = fix_type;
}

void SnologSentenceCollector::OnVelocityPacket(int time_of_week_ms, int north_vel, int east_vel, int down_vel, int speed_3d, int speed_2d, int heading)
{
	if (time_of_week_ms != m_last_time_of_week)
	{
		// Then it's a new bunch of packets, process the accumulated state.
		ProcessAccumulatedState();
		m_last_time_of_week = time_of_week_ms;
	}

	//		TRACE("time_of_week_ms = %d ms\n", time_of_week_ms);
	//		TRACE("north_vel = %d cm/s\n", north_vel);
	//		TRACE("east_vel = %d cm/s\n", east_vel);
	//		TRACE("down_vel = %d cm/s\n", down_vel);
	//		TRACE("speed_3d = %d cm/s\n", speed_3d);
	//		TRACE("speed_2d = %d cm/s\n", speed_2d);
	//		TRACE("heading = %d\n", heading);

	if (speed_3d > m_max_speed_3d)
		m_max_speed_3d = speed_3d;
}

void SnologSentenceCollector::OnTemperaturePacket(int time_of_week_ms, int temperature)
{
	if (time_of_week_ms != m_last_time_of_week)
	{
		// Then it's a new bunch of packets, process the accumulated state.
		ProcessAccumulatedState();
		m_last_time_of_week = time_of_week_ms;
	}

	//		TRACE("time_of_week_ms = %d\n", time_of_week_ms);
	//		TRACE("temperature = %d\n", temperature);
}

void SnologSentenceCollector::OnPositionPacket(int time_of_week_ms, int longitude, int latitude, int height_above_ellipsoid, int height_above_msl, int horiz_accuracy, int vert_accuracy)
{
	if (time_of_week_ms != m_last_time_of_week)
	{
		// Then it's a new bunch of packets, process the accumulated state.
		ProcessAccumulatedState();
		m_last_time_of_week = time_of_week_ms;
	}

	//		TRACE("time_of_week_ms = %d ms\n", time_of_week_ms);
	//		TRACE("latitude = %d\n", latitude);
	//		TRACE("longiutde = %d\n", longitude);
	//		TRACE("height_above_ellipsoid = %d mm\n", height_above_ellipsoid);
	//		TRACE("height_above_msl = %d mm\n", height_above_msl);
	//		TRACE("horiz_accuracy = %d mm\n", horiz_accuracy);
	//		TRACE("vert_accuracy = %d mm\n", vert_accuracy);

	m_state.longitude = longitude;
	m_state.latitude = latitude;
}

void SnologSentenceCollector::OnAltitudePacket(int time_of_week_ms, int altitude)
{
	if (time_of_week_ms != m_last_time_of_week)
	{
		// Then it's a new bunch of packets, process the accumulated state.
		ProcessAccumulatedState();
		m_last_time_of_week = time_of_week_ms;
	}

	TRACE("time_of_week_ms = %d\n", time_of_week_ms);
	TRACE("altitude = %d\n", altitude);
}

void SnologSentenceCollector::ProcessAccumulatedState()
{
	if (m_last_time_of_week == -1)
		return;	// It's the first time change, so we don't have any info yet.

	if (m_state.fix_type >= 2 && (m_state.fix_flags & 0x0C))
	{
		double lon_deg = (double)m_state.longitude / 10000000;
		double lat_deg = (double)m_state.latitude / 10000000;
		FILETIME timestamp = GpsTimeToFileTime(m_state.gps_week_number, m_state.time_of_week_ms);
		double gps_elevation = 0.0;

		CPoint p = m_proj->ForwardDeg(lon_deg, lat_deg);

		PositionFix fix(m_trk_ident, lon_deg, lat_deg, timestamp, gps_elevation, false, p.x, p.y);
		m_positions->push_back(fix);
	}
}
