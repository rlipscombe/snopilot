#include "StdAfx.h"
#include "ImportSerialTask.h"
#include "ImportSerialSettings.h"
#include "PositionFix.h"
#include "Waypoint.h"

#include "serial_device.h"
#include "garmin/basic_link_protocol.h"
#include "garmin/garmin_device.h"
#include "garmin/garmin_packet_ids.h"
#include "garmin/garmin_semicircles.h"
#include "garmin/garmin_time.h"
#include "Projection.h"

#include "progress.h"

#define TRACE_TRACKLOG 0

class TrackLogCollector : public TrackLogCallback
{
	PositionFixCollection *m_positions;
	std::string m_trk_ident;
	Projection *m_proj;

public:
	TrackLogCollector(PositionFixCollection *positions, Projection *proj)
		: m_positions(positions), m_proj(proj)
	{
	}

// TrackLogCallback
public:
	virtual void OnTrackHeader(const D310_Trk_Hdr_Type &trk_hdr)
	{
		TRACEC(TRACE_TRACKLOG, "D310_Trk_Hdr_Type:\n");
		TRACEC(TRACE_TRACKLOG, "  trk_hdr.dspl      = %c\n", trk_hdr.dspl ? 'y' : 'n');
		TRACEC(TRACE_TRACKLOG, "  trk_hdr.color     = %d (0x%x) ('%s')\n", trk_hdr.color, trk_hdr.color, DescribeD310Color(trk_hdr.color).c_str());
		TRACEC(TRACE_TRACKLOG, "  trk_hdr.trk_ident = '%s'\n", trk_hdr.trk_ident.c_str());

		m_trk_ident = trk_hdr.trk_ident;
	}

	virtual void OnTrackPoint(const D301_Trk_Point_Type &trk_point)
	{
		TRACEC(TRACE_TRACKLOG, "D301_Trk_Point_Type:\n");
		TRACEC(TRACE_TRACKLOG, "  trk_point.posn.lat = %ld (0x%x) (%f)\n", trk_point.posn.lat, trk_point.posn.lat, DegreesFromSemicircles(trk_point.posn.lat));
		TRACEC(TRACE_TRACKLOG, "  trk_point.posn.lon = %ld (0x%x) (%f)\n", trk_point.posn.lon, trk_point.posn.lon, DegreesFromSemicircles(trk_point.posn.lon));
		TRACEC(TRACE_TRACKLOG, "  trk_point.time     = %lu (0x%x) ('%s')\n", trk_point.time, trk_point.time, FormatGarminTime(trk_point.time).c_str());
		TRACEC(TRACE_TRACKLOG, "  trk_point.alt      = %f m\n", trk_point.alt);
		TRACEC(TRACE_TRACKLOG, "  trk_point.dpth     = %f m\n", trk_point.dpth);
		TRACEC(TRACE_TRACKLOG, "  trk_point.new_trk  = %c\n", trk_point.new_trk ? 'y' : 'n');

		double lon_deg = DegreesFromSemicircles(trk_point.posn.lon);
		double lat_deg = DegreesFromSemicircles(trk_point.posn.lat);
		FILETIME timestamp = FileTimeFromGarminTime(trk_point.time);
		double gps_elevation = trk_point.alt;
		bool isNew = trk_point.new_trk;

		CPoint p = m_proj->ForwardDeg(lon_deg, lat_deg);

		PositionFix position(m_trk_ident.c_str(), lon_deg, lat_deg, timestamp, gps_elevation, isNew, p.x, p.y);

		m_positions->push_back(position);
	}
};

class TrackLogProgress : public Progress
{
	ImportSerialTask *m_pTask;

public:
	TrackLogProgress(ImportSerialTask *pTask)
		: m_pTask(pTask)
	{
	}

// Progress
public:
	virtual STATUS OnProgress(int num, int denom)
	{
		if (m_pTask->QueryCancelTask())
			return E_USER_CANCELLED;
		m_pTask->ReportTrackLogProgress(num, denom);
		return S_OK;
	}

	virtual STATUS OnComplete()
	{
		m_pTask->ReportTrackLogComplete();
		return S_OK;
	}
};

std::string StringFromFixedArray(const char *p, int n)
{
	return std::string(p, p + n);
}

class WaypointCollector : public WaypointCallback
{
	WaypointCollection *m_waypoints;
	Projection *m_proj;

public:
	WaypointCollector(WaypointCollection *waypoints, Projection *proj)
		: m_waypoints(waypoints), m_proj(proj)
	{
	}

// WaypointCallback
public:
	virtual void OnWaypoint(const D108_Wpt_Type &wpt)
	{
		TRACEC(TRACE_TRACKLOG, "D108_Wpt_Type:\n");
		TRACEC(TRACE_TRACKLOG, "  wpt.wpt_class = %d ('%s')\n", wpt.wpt_class, DescribeD108WaypointClass(wpt.wpt_class).c_str());
		TRACEC(TRACE_TRACKLOG, "  wpt.color = %d (0x%x) ('%s')\n", wpt.color, wpt.color, DescribeD108Color(wpt.color).c_str());
		TRACEC(TRACE_TRACKLOG, "  wpt.dspl = %d (0x%x) ('%s')\n", wpt.dspl, wpt.dspl, DescribeD103DisplayOptions(wpt.dspl).c_str());
		TRACEC(TRACE_TRACKLOG, "  wpt.attr = %d\n", wpt.attr);
		TRACEC(TRACE_TRACKLOG, "  wpt.smbl = %d (0x%x) ('%s')\n", wpt.smbl, wpt.smbl, DescribeSymbolType(wpt.smbl).c_str());
		TRACEC(TRACE_TRACKLOG, "  wpt.sublass = <binary>\n");
		TRACEC(TRACE_TRACKLOG, "  wpt.posn.lat = %ld (0x%x) (%f)\n", wpt.posn.lat, wpt.posn.lat, DegreesFromSemicircles(wpt.posn.lat));
		TRACEC(TRACE_TRACKLOG, "  wpt.posn.lon = %ld (0x%x) (%f)\n", wpt.posn.lon, wpt.posn.lon, DegreesFromSemicircles(wpt.posn.lon));
		TRACEC(TRACE_TRACKLOG, "  wpt.alt      = %f m\n", wpt.alt);
		TRACEC(TRACE_TRACKLOG, "  wpt.dpth     = %f m\n", wpt.dpth);
		TRACEC(TRACE_TRACKLOG, "  wpt.dist     = %f m\n", wpt.dist);
		TRACEC(TRACE_TRACKLOG, "  wpt.state    = '%s'\n", StringFromFixedArray(wpt.state, 2).c_str());
		TRACEC(TRACE_TRACKLOG, "  wpt.cc       = '%s'\n", StringFromFixedArray(wpt.cc, 2).c_str());
		TRACEC(TRACE_TRACKLOG, "  wpt.ident    = '%s'\n", wpt.ident.c_str());
		TRACEC(TRACE_TRACKLOG, "  wpt.comment    = '%s'\n", wpt.comment.c_str());
		TRACEC(TRACE_TRACKLOG, "  wpt.facility    = '%s'\n", wpt.facility.c_str());
		TRACEC(TRACE_TRACKLOG, "  wpt.city    = '%s'\n", wpt.city.c_str());
		TRACEC(TRACE_TRACKLOG, "  wpt.addr    = '%s'\n", wpt.addr.c_str());
		TRACEC(TRACE_TRACKLOG, "  wpt.cross_road    = '%s'\n", wpt.cross_road.c_str());

		double lon_deg = DegreesFromSemicircles(wpt.posn.lon);
		double lat_deg = DegreesFromSemicircles(wpt.posn.lat);

	    CPoint p = m_proj->ForwardDeg(lon_deg, lat_deg);
		Waypoint waypoint(wpt.ident.c_str(), lon_deg, lat_deg, p.x, p.y);
		m_waypoints->push_back(waypoint);
	}
};

class WaypointProgress : public Progress
{
	ImportSerialTask *m_pTask;

public:
	WaypointProgress(ImportSerialTask *pTask)
		: m_pTask(pTask)
	{
	}

// Progress
public:
	virtual STATUS OnProgress(int num, int denom)
	{
		if (m_pTask->QueryCancelTask())
			return E_FAIL;
		m_pTask->ReportWaypointProgress(num, denom);
		return S_OK;
	}

	virtual STATUS OnComplete()
	{
		m_pTask->ReportWaypointComplete();
		return S_OK;
	}
};

void ImportSerialTask::Run()
{
	ReportBegin();

	Win32SerialDevice device;
	if (!device.Open(m_pSettings->GetSerialDevice().c_str()))
	{
		ReportComplete(E_FAIL, NULL, NULL, NULL);
		return;
	}

	BasicLinkProtocol protocol(&device);
	GarminDevice garmin(&protocol);

	Product_Data_Type productData;
	STATUS status = garmin.GetProductData(&productData);
	if (FAILED(status))
	{
		ReportComplete(status, NULL, NULL, NULL);
		return;
	}

	TRACEC(TRACE_TRACKLOG, "    product_id = %d (0x%x)\n", productData.product_id, productData.product_id);
	TRACEC(TRACE_TRACKLOG, "    software_version = %d.%02d\n", productData.software_version / 100, productData.software_version % 100);
	TRACEC(TRACE_TRACKLOG, "    product_description = %s\n", productData.product_description.c_str());

	for (size_t i = 0; i < productData.additional_strings.size(); ++i)
		TRACEC(TRACE_TRACKLOG, "    more_strings = %s\n", productData.additional_strings[i].c_str());

	char *pargs[] = {
		"proj=merc", "ellps=WGS84",
	};

	Projection proj;
	proj.Create(COUNTOF(pargs), pargs);

	TRACE("Downloading track logs...\n");

	PositionFixCollection positions;
	TrackLogCollector trackLogCollector(&positions, &proj);

	TrackLogProgress trackLogProgress(this);
	status = garmin.GetTrackLogs(&trackLogCollector, &trackLogProgress);
	if (FAILED(status))
	{
		ReportComplete(status, NULL, NULL, NULL);
		return;
	}

	TRACE("Finished downloading track logs.\n");
	TRACE("Downloading waypoints...\n");

	WaypointCollection waypoints;
	WaypointCollector waypointCollector(&waypoints, &proj);

	WaypointProgress waypointProgress(this);
	garmin.GetWaypoints(&waypointCollector, &waypointProgress);

	TRACE("Finished downloading waypoints.\n");
	ReportComplete(S_OK, &productData, &positions, &waypoints);
}
