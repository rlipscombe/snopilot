/* nmea_sentence_collector.cpp
 *
 * Copyright (C) 2003 Snopilot, http://www.snopilot.com/
 */

#include "config.h"
#include "trace.h"
#include "nmea_sentence_collector.h"
#include "tokenise_string.h"
#include "gps/latitude.h"
#include "gps/longitude.h"

// For distances between two long/lat pairs,
// see http://www.mathforum.com/library/drmath/view/51711.html

bool NmeaSentenceCollector::ProcessBuffer(const char *buffer, int buffer_size, int *bytes_eaten)
{
	*bytes_eaten = 0;

	// Look through the combined buffer to see if there are any line breaks in there.
	bool result = true;

	const char *buffer_end = buffer + buffer_size;
	const char *b = buffer;
	const char *e;
	while((e = (const char *)memchr(b, '\n', buffer_end - b)) != NULL)
	{
		std::string line(b, e);
		b = e + 1;

		result = FireLine(line);
		if (!result)
			break;
	}

	*bytes_eaten = (int)(b - buffer);

	return result;
}

/** Remove Trailing CR. */
bool NmeaSentenceCollector::FireLine(const std::string &line)
{
	char last = *line.rbegin();
	if (last == '\r')
		return OnLine(std::string(line.begin(), line.begin() + line.size() - 1));
	else
		return OnLine(line);
}

bool NmeaSentenceCollector::OnLine(const std::string &sentence)
{
	// yyyymmddhhmmss.ddd
	SYSTEMTIME st;
	GetSystemTime(&st);
	char timestamp[MAX_PATH];
	sprintf(timestamp, "%04d/%02d/%02d %02d:%02d:%02d.%03d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
	
	printf("@ %s: %s\n", timestamp, sentence.c_str());

	/* From the FAQ:
        The data is transmitted in the form of "sentences".  Each
        sentence starts with a "$", a two letter "talker ID", a three
        letter "sentence ID", followed by a number of data fields
        separated by commas, and terminated by an optional checksum, and
        a carriage return/line feed.  A sentence may contain up to 82
        characters including the "$" and CR/LF.
	 */

	if (sentence[0] != '$')
	{
		// Then it's a malformed sentence -- ignore it for now.
		printf("  Malformed sentence: '%s'\n", sentence.c_str());

		/** @todo Keep a count of malformed sentences.  If it reaches a threshold, try to reset the serial port. */
	}
	else
	{
		std::string talker_id(sentence.substr(1, 2));
		std::string sentence_id(sentence.substr(3, 3));

//		printf("  talker_id   = %s\n", talker_id.c_str());
//		printf("  sentence_id = %s\n", sentence_id.c_str());

		// Some of the sentence types are proprietary, so we really need the combined talker/sentence ID to tell.
		std::string full_sentence_id(sentence.substr(1, 5));

		// The sentence data comes between the '$ABCDE,' and the '*', if any.
		std::string sentence_data = GetSentenceData(sentence);
//		printf("  sentence_data = %s\n", sentence_data.c_str());

		if (!VerifySentenceChecksum(sentence))
			return true;	// Skip this sentence.

		/** @todo Some sentence types have a mandatory checksum.  We need to check that we're receiving it in those cases. */

		tokens_t tokens = TokeniseString(sentence_data, ',');

//		for (tokens_t::const_iterator i = tokens.begin(); i != tokens.end(); ++i)
//			printf("  token: '%s'\n", i->c_str());

		// From the Geco 201, I've seen the following sentence IDs:
		// BOD, GGA, GLL, GSA, GSV, RMB, RMC, RME, RMZ, RTE.
		if (full_sentence_id == "GPBOD")
		{
			printf("  GPBOD - Bearing - origin to destination waypoint\n");
			/*
			
			BOD,045.,T,023.,M,DEST,START
				045.,T       bearing 045 True from "START" to "DEST"
				023.,M       breaing 023 Magnetic from "START" to "DEST"
				DEST         destination waypoint ID
				START        origin waypoint ID
			*/

			std::string bearing0(tokens[1] + tokens[2]);
			std::string bearing1(tokens[3] + tokens[4]);
			std::string dest_waypoint_id(tokens[5]);
			std::string orig_waypoint_id(tokens[6]);

			printf("    bearing0 = %s\n", bearing0.c_str());
			printf("    bearing1 = %s\n", bearing1.c_str());
			printf("    dest_waypoint_id = %s\n", dest_waypoint_id.c_str());
			printf("    orig_waypoint_id = %s\n", orig_waypoint_id.c_str());
		}
		else if (full_sentence_id == "GPGGA")
		{
			printf("  GPGGA - Global Positioning System Fix Data\n");
			/*
			$GPGGA,123519,4807.038,N,01131.324,E,1,08,0.9,545.4,M,46.9,M, , *42
			123519       Fix taken at 12:35:19 UTC
			4807.038,N   Latitude 48 deg 07.038' N
			01131.324,E  Longitude 11 deg 31.324' E
			1            Fix quality: 0 = invalid
										1 = GPS fix
										2 = DGPS fix
			08           Number of satellites being tracked
			0.9          Horizontal dilution of position
			545.4,M      Altitude, Metres, above mean sea level
			46.9,M       Height of geoid (mean sea level) above WGS84
							ellipsoid
			(empty field) time in seconds since last DGPS update
			(empty field) DGPS station ID number
			*/

			if (tokens.size() >= 15)
			{
				std::string fix_taken_at(tokens[1]);

				/** @todo Are the lat/long numbers negative, or are they 'S'/'W' */
				Latitude fix_latitude;
				fix_latitude.FromString(tokens[2] + tokens[3]);

				Longitude fix_longitude;
				fix_longitude.FromString(tokens[4] + tokens[5]);

				std::string fix_quality(tokens[6]);
				std::string satellite_count(tokens[7]);
				std::string horizontal_dilution(tokens[8]);
				std::string altitude_above_mean_sea_level(tokens[9] + tokens[10]);
				std::string height_of_geoid_above_wgs84(tokens[11] + tokens[12]);
				std::string time_since_last_dgps_update(tokens[13]);
				std::string dgps_station_id(tokens[14]);

				printf("    fix_taken_at                  = %s\n", fix_taken_at.c_str());
				printf("    fix_latitude                  = %s\n", fix_latitude.ToString().c_str());
				printf("    fix_longitude                 = %s\n", fix_longitude.ToString().c_str());
				printf("    fix_quality                   = %s\n", fix_quality.c_str());
				printf("    satellite_count               = %s\n", satellite_count.c_str());
				printf("    horizontal_dilution           = %s\n", horizontal_dilution.c_str());
				printf("    altitude_above_mean_sea_level = %s\n", altitude_above_mean_sea_level.c_str());
				printf("    height_of_geoid_above_wgs84   = %s\n", height_of_geoid_above_wgs84.c_str());
				printf("    time_since_last_dgps_update   = %s\n", time_since_last_dgps_update.c_str());
				printf("    dgps_station_id               = %s\n", dgps_station_id.c_str());
			}
		}
		else if (full_sentence_id == "GPGLL")
		{
			printf("  GPGLL - Geographic position, Latitude and Longitude\n");
			/*
			
			GLL,4916.45,N,12311.12,W,225444,A
			4916.46,N    Latitude 49 deg. 16.45 min. North
			12311.12,W   Longitude 123 deg. 11.12 min. West
			225444       Fix taken at 22:54:44 UTC
			A            Data valid
				(Garmin 65 does not include time and status)
			*/

			if (tokens.size() >= 7)
			{
				std::string latitude(tokens[1] + tokens[2]);
				std::string longitude(tokens[3] + tokens[4]);
				std::string fix_taken_at(tokens[5]);
				std::string data_valid(tokens[6]);

				printf("    latitude                  = %s\n", latitude.c_str());
				printf("    longitude                 = %s\n", longitude.c_str());
				printf("    fix_taken_at              = %s\n", fix_taken_at.c_str());
				printf("    data_valid                = %s\n", data_valid.c_str());
			}
		}
		else if (full_sentence_id == "GPGSA")
		{
			printf("  GPGSA - GPS DOP and active satellites\n");
			/*
			
			GSA,A,3,04,05,,09,12,,,24,,,,,2.5,1.3,2.1*39
			A            Auto selection of 2D or 3D fix (M = manual)
			3            3D fix
			04,05...     PRNs of satellites used for fix (space for 12)
			2.5          PDOP (dilution of precision)
			1.3          Horizontal dilution of precision (HDOP)
			2.1          Vertical dilution of precision (VDOP)
				DOP is an indication of the effect of satellite geometry on
				the accuracy of the fix.
			*/
		}
		else if (full_sentence_id == "GPGSV")
		{
			printf("  GPGSV - Satellites in view\n");
			/*
			
			GSV,2,1,08,01,40,083,46,02,17,308,41,12,07,344,39,14,22,228,45*75
			2            Number of sentences for full data
			1            sentence 1 of 2
			08           Number of satellites in view
			01           Satellite PRN number
			40           Elevation, degrees
			083          Azimuth, degrees
			46           Signal strength - higher is better
			<repeat for up to 4 satellites per sentence>
					There my be up to three GSV sentences in a data packet
			*/

			if (tokens.size() >= 4)
			{
				std::string sentence_count(tokens[1]);
				std::string sentence_index(tokens[2]);
				std::string satellites_in_view(tokens[3]);

				printf("    sentence_count     = %s\n", sentence_count.c_str());
				printf("    sentence_index     = %s\n", sentence_index.c_str());
				printf("    satellites_in_view = %s\n", satellites_in_view.c_str());
			}

			if (tokens.size() >= 8)
			{
				std::string satellite0_prn(tokens[4]);
				std::string satellite0_elevation_deg(tokens[5]);
				std::string satellite0_azimuth_deg(tokens[6]);
				std::string satellite0_signal_strength(tokens[7]);

				printf("    satellite0_prn             = %s\n", satellite0_prn.c_str());
				printf("    satellite0_elevation_deg   = %s\n", satellite0_elevation_deg.c_str());
				printf("    satellite0_azimuth_deg     = %s\n", satellite0_azimuth_deg.c_str());
				printf("    satellite0_signal_strength = %s\n", satellite0_signal_strength.c_str());
			}

			if (tokens.size() >= 12)
			{
				std::string satellite1_prn(tokens[8]);
				std::string satellite1_elevation_deg(tokens[9]);
				std::string satellite1_azimuth_deg(tokens[10]);
				std::string satellite1_signal_strength(tokens[11]);

				printf("    satellite1_prn             = %s\n", satellite1_prn.c_str());
				printf("    satellite1_elevation_deg   = %s\n", satellite1_elevation_deg.c_str());
				printf("    satellite1_azimuth_deg     = %s\n", satellite1_azimuth_deg.c_str());
				printf("    satellite1_signal_strength = %s\n", satellite1_signal_strength.c_str());
			}

			if (tokens.size() >= 16)
			{
				std::string satellite2_prn(tokens[12]);
				std::string satellite2_elevation_deg(tokens[13]);
				std::string satellite2_azimuth_deg(tokens[14]);
				std::string satellite2_signal_strength(tokens[15]);

				printf("    satellite2_prn             = %s\n", satellite2_prn.c_str());
				printf("    satellite2_elevation_deg   = %s\n", satellite2_elevation_deg.c_str());
				printf("    satellite2_azimuth_deg     = %s\n", satellite2_azimuth_deg.c_str());
				printf("    satellite2_signal_strength = %s\n", satellite2_signal_strength.c_str());
			}

			if (tokens.size() >= 20)
			{
				std::string satellite3_prn(tokens[16]);
				std::string satellite3_elevation_deg(tokens[17]);
				std::string satellite3_azimuth_deg(tokens[18]);
				std::string satellite3_signal_strength(tokens[19]);

				printf("    satellite3_prn             = %s\n", satellite3_prn.c_str());
				printf("    satellite3_elevation_deg   = %s\n", satellite3_elevation_deg.c_str());
				printf("    satellite3_azimuth_deg     = %s\n", satellite3_azimuth_deg.c_str());
				printf("    satellite3_signal_strength = %s\n", satellite3_signal_strength.c_str());
			}
		}
		else if (full_sentence_id == "GPRMB")
		{
			printf("  GPRMB - Recommended minimum navigation information (sent by nav. receiver when a destination waypoint is active)\n");
			/*
			RMB,A,0.66,L,003,004,4917.24,N,12309.57,W,001.3,052.5,000.5,V*0B
			A            Data status A = OK, V = warning
			0.66,L       Cross-track error (nautical miles, 9.9 max.),
									steer Left to correct (or R = right)
			003          Origin waypoint ID
			004          Destination waypoint ID
			4917.24,N    Destination waypoint latitude 49 deg. 17.24 min. N
			12309.57,W   Destination waypoint longitude 123 deg. 09.57 min. W
			001.3        Range to destination, nautical miles
			052.5        True bearing to destination
			000.5        Velocity towards destination, knots
			V            Arrival alarm  A = arrived, V = not arrived
			*0B          mandatory checksum
			*/
		}
		else if (full_sentence_id == "GPRMC")
		{
			printf("  GPRMC - Recommended minimum specific GPS/Transit data\n");
			/*
			
			RMC,225446,A,4916.45,N,12311.12,W,000.5,054.7,191194,020.3,E*68
			225446       Time of fix 22:54:46 UTC
			A            Navigation receiver warning A = OK, V = warning
			4916.45,N    Latitude 49 deg. 16.45 min North
			12311.12,W   Longitude 123 deg. 11.12 min West
			000.5        Speed over ground, Knots
			054.7        Course Made Good, True
			191194       Date of fix  19 November 1994
			020.3,E      Magnetic variation 20.3 deg East
			*68          mandatory checksum
			*/
		}
		else if (full_sentence_id == "PGRME")
		{
			printf("  PGRME - Position Error, metres\n");

			/*
			$PGRME,15.0,M,45.0,M,25.0,M*22
			15.0,M       Estimated horizontal position error in metres (HPE)
			45.0,M       Estimated vertical error (VPE) in metres
			25.0,M       Overall spherical equivalent position error
			*/

			if (tokens.size() >= 7)
			{
				std::string estimated_horizontal_pos_error(tokens[1] + tokens[2]);
				std::string estimated_vertical_pos_error(tokens[3] + tokens[4]);
				std::string overall_spherical_equivalent_pos_error(tokens[5] + tokens[6]);

				printf("    estimated_horizontal_pos_error         = %s\n", estimated_horizontal_pos_error.c_str());
				printf("    estimated_vertical_pos_error           = %s\n", estimated_vertical_pos_error.c_str());
				printf("    overall_spherical_equivalent_pos_error = %s\n", overall_spherical_equivalent_pos_error.c_str());
			}
		}
		else if (full_sentence_id == "PGRMZ")
		{
			printf("  PGRMZ - Altitude\n");
			/*
			$PGRMZ,93,f,3*21
			93,f         Altitude in feet
			3            Position fix dimensions 2 = user altitude
													3 = GPS altitude
			This sentence shows in feet, regardless of units shown on the display.
			*/

			if (tokens.size() >= 4)
			{
				std::string altitude(tokens[1] + tokens[2]);
				std::string dimensions(tokens[3]);

				printf("    altitude = %s\n", altitude.c_str());
				printf("    dimensions = %s\n", dimensions.c_str());
			}
		}
		else if (full_sentence_id == "GPRTE")
		{
			printf("  GPRTE - Waypoints in active route\n");
			/*
			RTE - Waypoints in active route
			RTE,2,1,c,0,W3IWI,DRIVWY,32CEDR,32-29,32BKLD,32-I95,32-US1,BW-32,BW-198*69
			2            two sentences for full data
			1            this is sentence 1 of 2
			c            c = complete list of waypoints in this route
							w = first listed waypoint is start of current leg
			0            Route identifier
			W3IWI...     Waypoint identifiers
			*/
		}
	}
	return true;	// We want some more lines.
}

int NmeaSentenceCollector::CalculateSentenceChecksum(const std::string &sentence) const
{
	/* From the FAQ:
		The optional checksum field consists of a "*" and two hex digits
		representing the exclusive OR of all characters between, but not
		including, the "$" and "*".  A checksum is required on some
		sentences.
	*/

	typedef std::string::size_type size_type;

	size_type start_checksum_pos = sentence.find('$');
	size_type end_checksum_pos = sentence.find('*');

	int expected_checksum = 0;
	for (size_type i = start_checksum_pos + 1; i < end_checksum_pos; ++i)
	{
//		printf("  %d : %c\n", i, sentence[i]);
		expected_checksum ^= sentence[i];
	}

	return expected_checksum;
}

bool NmeaSentenceCollector::VerifySentenceChecksum(const std::string &sentence) const
{
	std::string checksum;

	std::string::size_type checksum_pos = sentence.find('*');
	if (checksum_pos == std::string::npos)
		return true;	// There's no checksum: assume it's OK.

	checksum = sentence.substr(checksum_pos + 1);
//	printf("  checksum      = %s\n", checksum.c_str());

	int expected_checksum = CalculateSentenceChecksum(sentence);
//	printf("  expected_checksum = %02x\n", expected_checksum);
	
	int actual_checksum = strtol(checksum.c_str(), NULL, 16);
//	printf("  actual_checksum = %02x\n", actual_checksum);

	if (actual_checksum != expected_checksum)
	{
		printf("Invalid checksum: %s (expected %02X)\n", sentence.c_str(), expected_checksum);
		return false;
	}

	return true;
}

std::string NmeaSentenceCollector::GetSentenceData(const std::string &sentence) const
{
	std::string sentence_data;

	std::string::size_type checksum_pos = sentence.find('*');
	if (checksum_pos != std::string::npos)
	{
		// There's a checksum
		sentence_data = sentence.substr(6, checksum_pos - 6);
	}
	else
	{
		// There's no checksum
		sentence_data = sentence.substr(6);
	}

	return sentence_data;
}