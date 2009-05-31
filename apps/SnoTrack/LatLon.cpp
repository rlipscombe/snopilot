#include "StdAfx.h"
#include "LatLon.h"

inline char GetHemisphere(double deg, char pos_hemi, char neg_hemi)
{
	return (deg < 0.0) ? neg_hemi : pos_hemi;
}

void GetDegreesFractionalMinutesFromFractionalDegrees(double fractional_deg, int *degrees, double *minutes)
{
	double temp = fabs(fractional_deg);

	// Get a whole number of degrees:
	*degrees = (int)floor(temp);
	ASSERT(*degrees >= 0 && *degrees <= 180);

	temp -= *degrees;
	temp *= 60;

	*minutes = temp;
}

// We ought to use the proper Unicode code points for these.
const char DEGREE_SYMBOL = '\xB0';
const char MINUTE_SYMBOL = '\'';
const char SECOND_SYMBOL = '\"';

/*
 * [-/+/] ddd.xxxxxx
 * [-/+/] ddd:mm.xxxxx
 * [-/+/] ddd:mm:ss.xxx
 */
CString FormatLongitude(double lon_deg, int format)
{
	if (format == FORMAT_LON_GARMIN)
	{
		char hemisphere = GetHemisphere(lon_deg, 'E', 'W');
		
		int degrees;
		double minutes;
		GetDegreesFractionalMinutesFromFractionalDegrees(lon_deg, &degrees, &minutes);

		CString Hdd_mm_mmm;
		Hdd_mm_mmm.Format("%c%03d%c%06.3f%c", hemisphere, degrees, DEGREE_SYMBOL, minutes, MINUTE_SYMBOL);

		return Hdd_mm_mmm;
	}
	else if (format == FORMAT_LON_COLONS)
	{
		// [-/+/] ddd:mm.xxxxx
		int degrees;
		double minutes;
		GetDegreesFractionalMinutesFromFractionalDegrees(lon_deg, &degrees, &minutes);

		CString str;

		if (lon_deg < 0)
            str.Format("-%d:%06.3f", degrees, minutes);
		else
			str.Format("%d:%06.3f", degrees, minutes);

		return str;
	}
	else
		return "";
}

CString FormatLatitude(double lat_deg, int format)
{
	if (format == FORMAT_LAT_GARMIN)
	{
		char hemisphere = GetHemisphere(lat_deg, 'N', 'S');
		
		int degrees;
		double minutes;
		GetDegreesFractionalMinutesFromFractionalDegrees(lat_deg, &degrees, &minutes);

		CString Hdd_mm_mmm;
		Hdd_mm_mmm.Format("%c %02d%c%06.3f%c", hemisphere, degrees, DEGREE_SYMBOL, minutes, MINUTE_SYMBOL);

		return Hdd_mm_mmm;
	}
	else if (format == FORMAT_LAT_COLONS)
	{
		// [-/+/] ddd:mm.xxxxx
		int degrees;
		double minutes;
		GetDegreesFractionalMinutesFromFractionalDegrees(lat_deg, &degrees, &minutes);

		CString str;

		if (lat_deg < 0)
            str.Format("-%d:%06.3f", degrees, minutes);
		else
			str.Format("%d:%06.3f", degrees, minutes);

		return str;
	}
	else
		return "";
}

CString FormatLatLonPosition(double lat_deg, double lon_deg)
{
	CString lat = FormatLatitude(lat_deg, FORMAT_LAT_GARMIN);
	CString lon = FormatLongitude(lon_deg, FORMAT_LON_GARMIN);

	CString s;
	s.Format("%s %s", LPCTSTR(lat), LPCTSTR(lon));
	return s;
}

/** Parse a deg/min/sec value expressed as:
 * [-/+/] ddd.xxxxxx
 * [-/+/] ddd:mm.xxxxx
 * [-/+/] ddd:mm:ss.xxx
 */
static bool ParseLatLon(const char *str, int max_deg, double *deg)
{
	// Some basic sanity checks.
	ASSERT(str);
	ASSERT(deg);

	int sign = 1;
	if (*str == '-')
	{
		sign = -1;
		str++;
	}

	char *p;
	double degrees = strtod(str, &p);
	if (*p)
	{
		if (*p != ':')
			return false;

		char *q;
		double minutes = strtod(p+1, &q);

		degrees += minutes / 60;

		if (*q)
		{
			if (*q != ':')
				return false;

			char *r;
			double seconds = strtod(q+1, &r);

			degrees += seconds / 3600;
		}
	}

	*deg = sign * degrees;
	return true;
}

bool ParseLongitude(const char *str, double *lon_deg)
{
	return ParseLatLon(str, 180, lon_deg);
}

bool ParseLatitude(const char *str, double *lat_deg)
{
	return ParseLatLon(str, 90, lat_deg);
}

double GetLongitudeFromNMEAString(const std::string &lon, const std::string &hemi)
{
	// lon is given as "12311.12", meaning 123 degrees, 11.12 minutes.
	// hemi is given as "E" or "W".

	// The values are zero prefixed, and may have more than 2 decimal places in the minutes.
	// The first part is easy, because we know that we've got exactly 3 digits to deal with:
	const int DEGREE_DIGITS = 3;
	std::string degs(lon.begin(), lon.begin() + DEGREE_DIGITS);
	std::string mins(lon.begin() + DEGREE_DIGITS, lon.end());

	double ddegs = strtod(degs.c_str(), NULL);
	double dmins = strtod(mins.c_str(), NULL);

	double value = ddegs + (dmins/60.0);
	if (hemi == "E")
		return value;
	else
		return -value;
}

double GetLatitudeFromNMEAString(const std::string &lat, const std::string &hemi)
{
	const int DEGREE_DIGITS = 2;
	std::string degs(lat.begin(), lat.begin() + DEGREE_DIGITS);
	std::string mins(lat.begin() + DEGREE_DIGITS, lat.end());

	double ddegs = strtod(degs.c_str(), NULL);
	double dmins = strtod(mins.c_str(), NULL);

	double value = ddegs + (dmins/60.0);
	if (hemi == "N")
		return value;
	else
		return -value;
}
