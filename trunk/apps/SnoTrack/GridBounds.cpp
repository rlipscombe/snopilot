#include "StdAfx.h"
#include "GridBounds.h"

GridBounds GridBounds::Expand(double lon, double lat)
{
	double minLon = RoundDown(m_minLongitude, lon);
	double maxLon = RoundUp(m_maxLongitude, lon);
	double minLat = RoundDown(m_minLatitude, lat);
	double maxLat = RoundUp(m_maxLatitude, lat);

	return GridBounds(minLon, maxLon, minLat, maxLat);
}

#if defined(_DEBUG)
#include "test/test_assert.h"

void test_RoundUp()
{
	// If I've got 0.6 and I want to round up to the nearest 1.0, then I'll get 1.0:
	assertDoublesEqual(1.0, RoundUp(0.6, 1.0), DBL_EPSILON);
	
	// If I've got -0.6 and I want to round up to the nearest 1.0, then I'll get 0.0:
	assertDoublesEqual(0.0, RoundUp(-0.6, 1.0), DBL_EPSILON);
}

void test_RoundDown()
{
	// If I've got 0.6 and I want to round down to the nearest 1.0, then I'll get 0.0:
	assertDoublesEqual(0.0, RoundDown(0.6, 1.0), DBL_EPSILON);

	// If I've got -0.6 and I want to round down to the nearest 1.0, then I'll get -1.0:
	assertDoublesEqual(-1.0, RoundDown(-0.6, 1.0), DBL_EPSILON);
}

void test_GridBounds()
{
	// Let's start with something simple: a box centered on (0,0), that needs to be rounded to unit size.
	{
		GridBounds bounds(-0.7, 0.3, -0.2, 0.6);
		GridBounds result = bounds.Expand(1.0, 1.0);

		assertDoublesEqual(-1.0, result.m_minLongitude, DBL_EPSILON);
		assertDoublesEqual(1.0, result.m_maxLongitude, DBL_EPSILON);
		assertDoublesEqual(-1.0, result.m_minLatitude, DBL_EPSILON);
		assertDoublesEqual(1.0, result.m_maxLatitude, DBL_EPSILON);
	}

	// Something a little more complicated.
	{
		GridBounds bounds(6.8923497, 7.0648743, 45.4175818, 45.4701641);
		GridBounds result = bounds.Expand(1.0/60.0, 1.0/60.0);

		assertDoublesEqual(6.883333, result.m_minLongitude, 0.00001);
		assertDoublesEqual(7.066667, result.m_maxLongitude, 0.00001);
		assertDoublesEqual(45.41667, result.m_minLatitude, 0.00001);
		assertDoublesEqual(45.48333, result.m_maxLatitude, 0.00001);
	}
}
#endif
