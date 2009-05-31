#include "StdAfx.h"
#include "unit_tests.h"
#if defined(_DEBUG)
#include "test/test_assert.h"
#include "win32/VersionInfo.h"
#include "win32/ApplicationProfile.h"
#include "ImportSerialSettings.h"
#include "LatLon.h"
#include "unix_time.h"
#include "gps_time.h"
#include "GridBounds.h"
#include <map>
#include <algorithm>
#include "BspNode.h"
#include "win32_ui/PenCache.h"
#include "HotspotGrid.h"

void test_CVersionInfo();
void test_MemoryApplicationProfile();
void test_CreateBSP();
void test_InsertLineBSP();
void test_SearchBSP();

void test_ImportSerialSettings()
{
	// Do some testing with the string manipulation.
	ImportSerialSettings a(NULL);

	a.SetSerialDevice(_T("COM1"));

	assertStringsEqual(_T("\\\\.\\COM1"), a.GetSerialDevice());
	assertStringsEqual(_T("COM1"), a.GetUserSerialDeviceName());

	a.SetSerialDevice(_T("COM2"));

	assertStringsEqual(_T("\\\\.\\COM2"), a.GetSerialDevice());
	assertStringsEqual(_T("COM2"), a.GetUserSerialDeviceName());

	MemoryApplicationProfile profile;

	a.Save(&profile);

	ImportSerialSettings b(NULL);
	b.Load(&profile);

	assertStringsEqual(_T("\\\\.\\COM2"), b.GetSerialDevice());
	assertStringsEqual(_T("COM2"), b.GetUserSerialDeviceName());
}

void test_FormatLongitude()
{
	assertStringsEqual("E000°00.000'", FormatLongitude(0.0, FORMAT_LON_GARMIN));
	assertStringsEqual("E090°00.000'", FormatLongitude(90.0, FORMAT_LON_GARMIN));
	assertStringsEqual("E180°00.000'", FormatLongitude(180.0, FORMAT_LON_GARMIN));
	assertStringsEqual("W090°00.000'", FormatLongitude(-90.0, FORMAT_LON_GARMIN));
	assertStringsEqual("W180°00.000'", FormatLongitude(-180.0, FORMAT_LON_GARMIN));
	assertStringsEqual("W000°08.315'", FormatLongitude(-0.138583, FORMAT_LON_GARMIN));
	assertStringsEqual("-0:08.315", FormatLongitude(-0.138583, FORMAT_LON_COLONS));
}

void test_FormatLatitude()
{
	assertStringsEqual("N 00°00.000'", FormatLatitude(0.0, FORMAT_LAT_GARMIN));
	assertStringsEqual("N 90°00.000'", FormatLatitude(90.0, FORMAT_LAT_GARMIN));
	assertStringsEqual("S 90°00.000'", FormatLatitude(-90.0, FORMAT_LAT_GARMIN));
	assertStringsEqual("N 51°34.314'", FormatLatitude(51.571903, FORMAT_LAT_GARMIN));
	assertStringsEqual("51:34.314", FormatLatitude(51.571903, FORMAT_LAT_COLONS));
}

/** Parse a deg/min/sec value expressed as:
 * [-/+/] ddd.xxxxxx
 * [-/+/] ddd:mm.xxxxx
 * [-/+/] ddd:mm:ss.xxx
 */
void test_ParseLongitude()
{
	double degs;

	assertTrue(ParseLongitude("123.138583", &degs));
	assertDoublesEqual(123.138583, degs, 0.00001);

	assertTrue(ParseLongitude("-123.138583", &degs));
	assertDoublesEqual(-123.138583, degs, 0.00001);

	assertTrue(ParseLongitude("-000:08.315", &degs));
	assertDoublesEqual(-0.138583, degs, 0.00001);

	assertTrue(ParseLongitude("123:08.315", &degs));
	assertDoublesEqual(123.138583, degs, 0.00001);

	assertTrue(ParseLongitude("-000:08:19", &degs));
	assertDoublesEqual(-0.138611, degs, 0.00001);

	assertFalse(ParseLongitude("X000:08:19", &degs));
}

void test_MakeUnixTime()
{
	assertIntegersEqual(1085356800, (int)MakeUnixTime(2004, 5, 24, 0, 0, 0));
}

/** Tests for converting between GPS week number + millisecond of week into Unix time_t, e.g.
 */
void test_GpsTime()
{
	// snolog gives us packet 2 containing the week number and the time of the week.
	// e.g. 2,10F67BC8,04F2,00,6C,00
	// We need to turn it into a time_t or something.

	// 04F2 (1266) is the week containing 14th April 2004.
	// See http://www.colorado.edu/geography/gcraft/notes/gps/gpseow.htm for more information.

	int time_of_week_ms = 0x10F67BC8;
	int gps_week_number = 0x04F2;

	// 0x04F2 (1266) is the week beginning with 11th April, 2004.
	time_t u = MakeUnixTime(2004, 4, 11, 0, 0, 0);
	time_t g = GpsWeekNumberToUnixTime(gps_week_number);
	assertIntegersEqual((int)u, (int)g);

	// Now, turn that into a timestamp, so we can see what it is.
	time_t u2 = MakeUnixTime(2004, 4, 14, 7, 3, 9);
	time_t g2 = GpsTimeToUnixTime(gps_week_number, time_of_week_ms);
	assertIntegersEqual((int)u2, (int)g2);
}

void test_Nmea()
{
	// First up, test that we can correctly parse the lon/lat from an NMEA sentence -- they come
	// as a pair of fields:
	//		4916.45,N    Latitude 49 deg. 16.45 min North
	//		12311.12,W   Longitude 123 deg. 11.12 min West
	assertDoublesEqual(-123.18533, GetLongitudeFromNMEAString("12311.12", "W"), 0.00001);
	assertDoublesEqual(49.27417, GetLatitudeFromNMEAString("4916.45", "N"), 0.00001);

	assertDoublesEqual(51.58239, GetLatitudeFromNMEAString("5134.9435", "N"), 0.00001);
	assertDoublesEqual(-0.15014, GetLongitudeFromNMEAString("00009.0084", "W"), 0.00001);

	// Then check that we can correctly parse a GPRMC sentence into its constituent parts.
}

void test_CreatePenPerformance()
{
	LARGE_INTEGER startCount;
	VERIFY(QueryPerformanceCounter(&startCount));

	const int MAX_ITERATIONS = 100000;
	for (int i = 0; i < MAX_ITERATIONS; ++i)
	{
		CPen pen;
		pen.CreatePen(PS_SOLID, 1, RGB(0,0,0));
	}

	LARGE_INTEGER endCount;
	VERIFY(QueryPerformanceCounter(&endCount));

	LARGE_INTEGER freq;
	VERIFY(QueryPerformanceFrequency(&freq));

	INT64 counts = endCount.QuadPart - startCount.QuadPart;
	
	double seconds = (double)counts / freq.QuadPart;
	double each = seconds / MAX_ITERATIONS;

	TRACE("Creating a black pen takes %f ms.\n", each * 1000);
}

void test_SelectPenPerformance()
{
	CWindowDC screenDC(NULL);

	CDC memoryDC;
	VERIFY(memoryDC.CreateCompatibleDC(&screenDC));

	CPen pen;
	pen.CreatePen(PS_SOLID, 1, RGB(0,0,0));

	LARGE_INTEGER startCount;
	VERIFY(QueryPerformanceCounter(&startCount));

	const int MAX_ITERATIONS = 100000;
	for (int i = 0; i < MAX_ITERATIONS; ++i)
	{
		CPen *oldPen = memoryDC.SelectObject(&pen);
		memoryDC.SelectObject(oldPen);
	}

	LARGE_INTEGER endCount;
	VERIFY(QueryPerformanceCounter(&endCount));

	LARGE_INTEGER freq;
	VERIFY(QueryPerformanceFrequency(&freq));

	INT64 counts = endCount.QuadPart - startCount.QuadPart;
	
	double seconds = (double)counts / freq.QuadPart;
	double each = seconds / MAX_ITERATIONS;

	TRACE("Selecting/deselecting a black pen takes %f ms.\n", each * 1000);
}

void test_PenCache()
{
	// Because creating/selecting pens takes time, we can optimise the creation
	// step by using a pen cache.
	
	PenCache penCache;

	// Try to get a solid black pen, width 1.
	CPen *solidBlackPen = penCache.GetPen(PS_SOLID, 1, RGB(0,0,0));
	assertTrue(solidBlackPen != NULL);

	LOGPEN blackPenObject;
	solidBlackPen->GetLogPen(&blackPenObject);

	assertIntegersEqual(PS_SOLID, blackPenObject.lopnStyle);
	assertIntegersEqual(1, blackPenObject.lopnWidth.x);
	assertIntegersEqual(RGB(0,0,0), blackPenObject.lopnColor);

	// Try to get a solid red pen, width 1.
	CPen *solidRedPen = penCache.GetPen(PS_SOLID, 1, RGB(255,0,0));
	assertTrue(solidRedPen != NULL);

	LOGPEN redPenObject;
	solidRedPen->GetLogPen(&redPenObject);

	assertIntegersEqual(PS_SOLID, redPenObject.lopnStyle);
	assertIntegersEqual(1, redPenObject.lopnWidth.x);
	assertIntegersEqual(RGB(255,0,0), redPenObject.lopnColor);

	assertTrue(solidRedPen != solidBlackPen);
}

int runUnitTests()
{
	test_CreatePenPerformance();
	test_SelectPenPerformance();

	test_PenCache();

	test_HotspotGrid();

	test_CVersionInfo();
	test_MemoryApplicationProfile();

	test_CreateBSP();
	test_InsertLineBSP();
	test_SearchBSP();

	test_ImportSerialSettings();
	test_FormatLongitude();
	test_FormatLatitude();
	test_ParseLongitude();

	test_MakeUnixTime();
	test_GpsTime();

	test_Nmea();

	test_RoundUp();
	test_RoundDown();
	test_GridBounds();

	return assertSummarise();
}
#endif
