#pragma once

/** This represents the area covered by the entire data set.
 */
struct GridBounds
{
	double m_minLongitude;
	double m_maxLongitude;
	double m_minLatitude;
	double m_maxLatitude;

	GridBounds()
		: m_minLongitude(0), m_maxLongitude(0), m_minLatitude(0), m_maxLatitude(0)
	{
	}

	GridBounds(double minLon, double maxLon, double minLat, double maxLat)
		: m_minLongitude(minLon), m_maxLongitude(maxLon), m_minLatitude(minLat), m_maxLatitude(maxLat)
	{
	}

	/** Expand the grid bounds so that the min/max lat/lon are multiples of lon/lat given here.
	 */
	GridBounds Expand(double lon, double lat);
};

inline double RoundUp(double x, double w)
{
	// How many times does w go into x?
	int y = (int)(x/w);

	if (x >= 0.0)
	{
		// We need one more of them.
		return (y + 1) * w;
	}
	else
	{
		return y * w;
	}
}

inline double RoundDown(double x, double w)
{
	// How many times does w go into x?
	int y = (int)(x/w);

	if (x >= 0.0)
	{
		// We need exactly that many of them.
		return y * w;
	}
	else
	{
		return (y - 1) * w;
	}
}

inline int RoundUp(int x, int w)
{
	// How many times does w go into x?
	int y = x/w;

	if (x >= 0)
	{
		// It goes exactly.  Leave it alone.
		if ((x % w) == 0)
			return x;
		
		// We need one more of them.
		return (y + 1) * w;
	}
	else
	{
		return y * w;
	}
}

inline int RoundDown(int x, int w)
{
	// How many times does w go into x?
	int y = x/w;

	if (x >= 0)
	{
		// We need exactly that many of them.
		return y * w;
	}
	else
	{
		// It goes exactly.  Leave it alone.
		if ((x % w) == 0)
			return x;

		return (y - 1) * w;
	}
}

#if defined(_DEBUG)
void test_RoundUp();
void test_RoundDown();
void test_GridBounds();
#endif
