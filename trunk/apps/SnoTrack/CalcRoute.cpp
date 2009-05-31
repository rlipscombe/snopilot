#include "StdAfx.h"
#include "CalcRoute.h"
#include <algorithm>
#include "HotspotNames.h"

#ifdef _DEBUG
#include "win32/TimeZoneInformation.h"
#endif

class MatchHotspot
{
public:
	bool operator() (int lhs, const HotspotVisit &rhs)
	{
		return lhs == rhs.id;
	}
};

class MatchPath
{
	const HotspotVisitCollection::const_iterator m_h;
	const HotspotVisitCollection::const_iterator m_h_end;

public:
	MatchPath(const HotspotVisitCollection::const_iterator h, const HotspotVisitCollection::const_iterator h_end)
		: m_h(h), m_h_end(h_end)
	{
	}

	bool operator() (const HotspotPath &p)
	{
		if (p.size() < 2)
			return false;	// Avoid infinite loop below by silently ignoring paths that are too short.

		// The remainder of the visited hotspot collection isn't big enough that there can be a match.
		if (std::distance(m_h, m_h_end) < (int)p.size())
			return false;

		// Otherwise, compare the two ranges.
		return std::equal(p.begin(), p.end(), m_h, MatchHotspot());
	}
};

void CalculateRoute(const HotspotPathCollection &paths, HotspotNames hotspotNames, const HotspotVisitCollection &hotspotsVisited, RouteCallback *pCallback)
{
	// Starting at the beginning of the visited hotspots, see which paths were visited.
	HotspotVisitCollection::const_iterator last = hotspotsVisited.end();

	HotspotVisitCollection::const_iterator h_end = hotspotsVisited.end();
	for (HotspotVisitCollection::const_iterator h = hotspotsVisited.begin(); h != h_end; )
	{
		HotspotPathCollection::const_iterator p = std::find_if(paths.begin(), paths.end(), MatchPath(h, h_end));

		if (p != paths.end())
		{
			pCallback->AddMatchedPath(*p, h, h + p->size());
			h += (p->size() - 1);
			last = h;
		}
		else
		{
			if (last != h)
			{
#if defined(ENABLE_DESIGN_MODE)
				utf8_string name = hotspotNames[h->id];
				CString hotspotName = ConvertUTF8ToSystem(name).c_str();
				if (hotspotName.IsEmpty())
					hotspotName.Format("#%d", h->id);

				pCallback->AddUnmatchedHotspot(hotspotName, *h);
#endif
			}

			++h;
		}
	}
}

void RouteEventCollector::AddMatchedPath(const HotspotPath &path, HotspotVisitCollection::const_iterator h_begin, HotspotVisitCollection::const_iterator h_end)
{
	RouteEvent evt;
	evt.path = &path;
	evt.description = ConvertUTF8ToSystem(path.GetName()).c_str();

	// Assume that the timer starts from leaving the first spot, to entering the last.
	evt.startTime = h_begin->exitTime;
	
	HotspotVisitCollection::const_iterator h_last = h_end-1;
	evt.endTime = h_last->entryTime;

	evt.imageIndex = path.GetImageIndex();

/*#ifdef _DEBUG
	TIME_ZONE_INFORMATION tzi;
	GetRegistryTimeZoneInformation(_T("Pacific Standard Time"), &tzi);

	TRACE("AddMatchedPath:\n");
	TRACE("  description:   %s\n", LPCTSTR(evt.description));

	for (HotspotVisitCollection::const_iterator i = h_begin; i != h_end; ++i)
		TRACE("  Hotspot: %d: %s -- %s\n", i->id, LPCTSTR(FormatFileTimeMs(i->entryTime, &tzi)), LPCTSTR(FormatFileTimeMs(i->exitTime, &tzi)));
#endif*/

	std::copy(h_begin, h_end, std::back_inserter(evt.hotspots));

	m_vec->push_back(evt);
}

void RouteEventCollector::AddUnmatchedHotspot(CString hotspotName, const HotspotVisit &v)
{
/*#ifdef _DEBUG
	TIME_ZONE_INFORMATION tzi;
	GetRegistryTimeZoneInformation(_T("Pacific Standard Time"), &tzi);

	TRACE("AddUnmatchedHotspot:\n");
	TRACE("  Hotspot: %d: %s -- %s\n", v.id, LPCTSTR(FormatFileTimeMs(v.entryTime, &tzi)), LPCTSTR(FormatFileTimeMs(v.exitTime, &tzi)));
#endif*/
	
	RouteEvent evt;
	evt.path = NULL;
	evt.description = hotspotName;
	evt.startTime = v.entryTime;
	evt.endTime = v.exitTime;
	evt.imageIndex = v.imageIndex;
	evt.hotspots.push_back(v);

	m_vec->push_back(evt);
}
