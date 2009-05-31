/* dump_garmin.cpp
 *
 * Copyright (C) 2003 Snopilot, http://www.snopilot.com/
 */

#include "config.h"
#include "trace.h"
#include <stdio.h>
#include "serial_device.h"
#include "garmin/basic_link_protocol.h"
#include "garmin/garmin_device.h"
#include "garmin/garmin_packet_builder.h"
#include "garmin/garmin_packet_ids.h"
#include "progress.h"
#include "safe_snprintf.h"

extern int runTests_lib_garmin();

class CollectToFile
{
	HANDLE m_hFile;

public:
	CollectToFile()
		: m_hFile(INVALID_HANDLE_VALUE)
	{
	}

	~CollectToFile()
	{
		Close();
	}

	bool Create(const TCHAR *filename)
	{
		m_hFile = CreateFile(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
		if (m_hFile == INVALID_HANDLE_VALUE)
			return false;

		return true;
	}

	void Close()
	{
		if (m_hFile != INVALID_HANDLE_VALUE)
		{
			CloseHandle(m_hFile);
			m_hFile = NULL;
		}
	}

	BOOL Write(LPCVOID lpBuffer, DWORD nNumerOfBytesToWrite)
	{
		ASSERT(m_hFile != INVALID_HANDLE_VALUE);
		DWORD bytesWritten;
		return ::WriteFile(m_hFile, lpBuffer, nNumerOfBytesToWrite, &bytesWritten, NULL);
	}
};

/** Write the track log to a simple CSV file.
 */
class TrackLogCollector : public TrackLogCallback, public CollectToFile
{
	std::string m_trk_ident;

public:
	TrackLogCollector()
		: m_trk_ident("NONE")
	{
	}
	
	bool Create(const TCHAR *filename)
	{
		if (!CollectToFile::Create(filename))
			return false;

		const char csv_headings[] = "TRK_IDENT,LAT,LON,TIME,ALT,DPTH,NEW,LAT_DEG,LON_DEG,TIME_GMT\r\n";
		
		VERIFY(Write(csv_headings, (DWORD)strlen(csv_headings)));

		return true;
	}

// TrackLogCallback
public:
	virtual void OnTrackHeader(const D310_Trk_Hdr_Type &trk_hdr)
	{
//		TRACE("D310_Trk_Hdr_Type:\n");
//		TRACE("  trk_hdr.dspl      = %c\n", trk_hdr.dspl ? 'y' : 'n');
//		TRACE("  trk_hdr.color     = %d (0x%x) ('%s')\n", trk_hdr.color, trk_hdr.color, DescribeD310Color(trk_hdr.color).c_str());
//		TRACE("  trk_hdr.trk_ident = '%s'\n", trk_hdr.trk_ident.c_str());
		m_trk_ident = trk_hdr.trk_ident;
	}

	virtual void OnTrackPoint(const D301_Trk_Point_Type &trk_point)
	{
//		TRACE("D301_Trk_Point_Type:\n");
//		TRACE("  trk_point.posn.lat = %ld (0x%x) (%f)\n", trk_point.posn.lat, trk_point.posn.lat, DegreesFromSemicircles(trk_point.posn.lat));
//		TRACE("  trk_point.posn.lon = %ld (0x%x) (%f)\n", trk_point.posn.lon, trk_point.posn.lon, DegreesFromSemicircles(trk_point.posn.lon));
//		TRACE("  trk_point.time     = %lu (0x%x) ('%s')\n", trk_point.time, trk_point.time, FormatGarminTime(trk_point.time).c_str());
//		TRACE("  trk_point.alt      = %f m\n", trk_point.alt);
//		TRACE("  trk_point.dpth     = %f m\n", trk_point.dpth);
//		TRACE("  trk_point.new_trk  = %c\n", trk_point.new_trk ? 'y' : 'n');

		const char csv_record_fmt[] = "%s,%ld,%ld,%lu,%f,%f,%c,%f,%f,%s\r\n";

		float lat_deg = DegreesFromSemicircles(trk_point.posn.lat);
		float lon_deg = DegreesFromSemicircles(trk_point.posn.lon);
		std::string time_gmt = FormatGarminTime(trk_point.time);

		char csv_record[MAX_PATH];
		int csv_record_len = safe_snprintf(csv_record, sizeof(csv_record), csv_record_fmt,
											m_trk_ident.c_str(), trk_point.posn.lat, trk_point.posn.lon, trk_point.time,
											trk_point.alt, trk_point.dpth,
											(trk_point.new_trk ? 'Y' : 'N'),
											lat_deg, lon_deg, time_gmt.c_str());

		VERIFY(Write(csv_record, csv_record_len));
	}
};

class TrackLogProgress : public Progress
{
public:
	virtual STATUS OnProgress(int num, int denom)
	{
		if (denom != 0)
		{
			int pct = num * 100 / denom;
			printf("\rDownloading track log, record %d of %d, %d%%       ", num, denom, pct);
		}

		return S_OK;
	}

	virtual STATUS OnComplete()
	{
		printf("\n");
		return S_OK;
	}
};

std::string StringFromFixedArray(const char *p, int n)
{
	return std::string(p, p + n);
}

class WaypointCollector : public WaypointCallback, public CollectToFile
{
	HANDLE m_hFile;

public:
	WaypointCollector()
		: m_hFile(INVALID_HANDLE_VALUE)
	{
	}

	bool Create(const TCHAR *filename)
	{
		if (!CollectToFile::Create(filename))
			return false;

		const char csv_headings[] = "LAT,LON,LAT_DEG,LON_DEG,IDENT\r\n";
		
		VERIFY(Write(csv_headings, (DWORD)strlen(csv_headings)));

		return true;
	}
	
// WaypointCallback
public:
	virtual void OnWaypoint(const D108_Wpt_Type &wpt)
	{
//		TRACE("D108_Wpt_Type:\n");
//		TRACE("  wpt.wpt_class = %d ('%s')\n", wpt.wpt_class, DescribeD108WaypointClass(wpt.wpt_class).c_str());
//		TRACE("  wpt.color = %d (0x%x) ('%s')\n", wpt.color, wpt.color, DescribeD108Color(wpt.color).c_str());
//		TRACE("  wpt.dspl = %d (0x%x) ('%s')\n", wpt.dspl, wpt.dspl, DescribeD103DisplayOptions(wpt.dspl).c_str());
//		TRACE("  wpt.attr = %d\n", wpt.attr);
//		TRACE("  wpt.smbl = %d (0x%x) ('%s')\n", wpt.smbl, wpt.smbl, DescribeSymbolType(wpt.smbl).c_str());
//		TRACE("  wpt.sublass = <binary>\n");
//		TRACE("  wpt.posn.lat = %ld (0x%x) (%f)\n", wpt.posn.lat, wpt.posn.lat, DegreesFromSemicircles(wpt.posn.lat));
//		TRACE("  wpt.posn.lon = %ld (0x%x) (%f)\n", wpt.posn.lon, wpt.posn.lon, DegreesFromSemicircles(wpt.posn.lon));
//		TRACE("  wpt.alt      = %f m\n", wpt.alt);
//		TRACE("  wpt.dpth     = %f m\n", wpt.dpth);
//		TRACE("  wpt.dist     = %f m\n", wpt.dist);
//		TRACE("  wpt.state    = '%s'\n", StringFromFixedArray(wpt.state, 2).c_str());
//		TRACE("  wpt.cc       = '%s'\n", StringFromFixedArray(wpt.cc, 2).c_str());
//		TRACE("  wpt.ident    = '%s'\n", wpt.ident.c_str());
//		TRACE("  wpt.comment    = '%s'\n", wpt.comment.c_str());
//		TRACE("  wpt.facility    = '%s'\n", wpt.facility.c_str());
//		TRACE("  wpt.city    = '%s'\n", wpt.city.c_str());
//		TRACE("  wpt.addr    = '%s'\n", wpt.addr.c_str());
//		TRACE("  wpt.cross_road    = '%s'\n", wpt.cross_road.c_str());

		const char csv_record_fmt[] = "%d,%d,%f,%f,%s\r\n";

		float lat_deg = DegreesFromSemicircles(wpt.posn.lat);
		float lon_deg = DegreesFromSemicircles(wpt.posn.lon);

		char csv_record[MAX_PATH];
		int csv_record_len = safe_snprintf(csv_record, sizeof(csv_record), csv_record_fmt,
											wpt.posn.lat, wpt.posn.lon, lat_deg, lon_deg, wpt.ident.c_str());

		VERIFY(Write(csv_record, csv_record_len));
	}
};

class WaypointProgress : public Progress
{
public:
	virtual STATUS OnProgress(int num, int denom)
	{
		if (denom != 0)
		{
			int pct = num * 100 / denom;
			printf("\rDownloading waypoints, record %d of %d, %d%%       ", num, denom, pct);
		}

		return S_OK;
	}

	virtual STATUS OnComplete()
	{
		printf("\n");
		return S_OK;
	}
};

int main(int argc, char *argv[])
{
#ifdef _DEBUG
	int testResult = runTests_lib_garmin();
	if (testResult != 0)
	{
		fprintf(stderr, "runTests failed.\n");
		return testResult;
	}
#endif

	if (argc < 4)
	{
		fprintf(stderr, "Usage:\n\tdump_garmin serial-port track-log-output-file waypoint-output-file\n");
		return 1;
	}

	// Prevent the computer from powering off:
	EXECUTION_STATE oldExecutionState = SetThreadExecutionState(ES_SYSTEM_REQUIRED | ES_CONTINUOUS);

	char serial_device[MAX_PATH];
	sprintf(serial_device, "\\\\.\\%s", argv[1]);

	const char *tracklog_file = argv[2];
	const char *waypoint_file = argv[3];

	Win32SerialDevice device;
	if (!device.Open(serial_device))
	{
		fprintf(stderr, "Failed to open serial device '%s'.\n", serial_device);
		return 1;
	}

	BasicLinkProtocol protocol(&device);
	GarminDevice garmin(&protocol);

	Product_Data_Type productData;
	garmin.GetProductData(&productData);
	printf("    product_id = %d (0x%x)\n", productData.product_id, productData.product_id);
	printf("    software_version = %d.%02d\n", productData.software_version / 100, productData.software_version % 100);
	printf("    product_description = %s\n", productData.product_description.c_str());

	for (size_t i = 0; i < productData.additional_strings.size(); ++i)
		printf("    more_strings = %s\n", productData.additional_strings[i].c_str());

	printf("Downloading track logs...\n");

	TrackLogCollector trackLogCollector;
	if (!trackLogCollector.Create(tracklog_file))
	{
		fprintf(stderr, "Failed to open track log output file '%s'.\n", tracklog_file);
		return 1;
	}
	
	TrackLogProgress trackLogProgress;
	garmin.GetTrackLogs(&trackLogCollector, &trackLogProgress);

	printf("Finished downloading track logs.\n");

	printf("Downloading waypoints...\n");

	WaypointCollector waypointCollector;
	if (!waypointCollector.Create(waypoint_file))
	{
		fprintf(stderr, "Failed to open waypoint output file '%s'.\n", waypoint_file);
		return 1;
	}

	WaypointProgress waypointProgress;
	garmin.GetWaypoints(&waypointCollector, &waypointProgress);

	printf("Finished downloading waypoints.\n");
	
	return 0;
}
