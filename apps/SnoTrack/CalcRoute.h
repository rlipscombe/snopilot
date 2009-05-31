#pragma once
#include "HotspotPath.h"
#include "HotspotVisit.h"
#include "HotspotNames.h"

class RouteCallback
{
public:
	virtual void AddMatchedPath(const HotspotPath &path, HotspotVisitCollection::const_iterator h_begin, HotspotVisitCollection::const_iterator h_end) = 0;
	virtual void AddUnmatchedHotspot(CString hotspotName, const HotspotVisit &v) = 0;
};

void CalculateRoute(const HotspotPathCollection &paths, HotspotNames hotspotNames, const HotspotVisitCollection &hotspotsVisited, RouteCallback *pCallback);

struct RouteEvent
{
	CString description;

	const HotspotPath *path;
	FILETIME startTime;
	FILETIME endTime;
	int imageIndex;

	HotspotVisitCollection hotspots;
};
typedef std::vector< RouteEvent > RouteEventCollection;

class RouteEventCollector : public RouteCallback
{
	RouteEventCollection *m_vec;

public:
	RouteEventCollector(RouteEventCollection *vec)
		: m_vec(vec)
	{
	}

public:
	virtual void AddMatchedPath(const HotspotPath &path, HotspotVisitCollection::const_iterator h_begin, HotspotVisitCollection::const_iterator h_end);
	virtual void AddUnmatchedHotspot(CString hotspotName, const HotspotVisit &v);
};
