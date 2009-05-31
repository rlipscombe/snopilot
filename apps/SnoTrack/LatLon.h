#pragma once
#include <string>

/** Return longitude formatted as a string, according to the format chosen.
 */
CString FormatLongitude(double lon_deg, int format);
#define FORMAT_LON_GARMIN 1	// W000°08.315'
#define FORMAT_LON_COLONS 2 // -0:8.315

/** Return latitude formatted as a string, according to the format chosen.
 */
CString FormatLatitude(double lat_deg, int format);
#define FORMAT_LAT_GARMIN 1	// N 51°34.314'
#define FORMAT_LAT_COLONS 2	// 51:34.314

CString FormatLatLonPosition(double lat_deg, double lon_deg);

bool ParseLongitude(const char *str, double *lon_deg);
bool ParseLatitude(const char *str, double *lat_deg);

/** NMEA sentences (e.g. GPRMC) specify lon/lat as, e.g. "4916.45,N"  These
 * two functions will deal with this correctly.
 */
double GetLongitudeFromNMEAString(const std::string &lon, const std::string &lon_hemi);
double GetLatitudeFromNMEAString(const std::string &lat, const std::string &lat_hemi);
