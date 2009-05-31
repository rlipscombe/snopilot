#include "StdAfx.h"
#include "PathLayer.h"

#if defined(ENABLE_DESIGN_MODE)
void PathLayer::OnRecalc(const CRect &logicalRange)
{
	DWORD startTime = GetTickCount();

	DestroyBsp();

	CreateBsp(logicalRange);

	// Generate a bunch of LineDefs from the path information:
	HotspotPathCollection::const_iterator b = m_paths->begin();
	HotspotPathCollection::const_iterator e = m_paths->end();

	for (HotspotPathCollection::const_iterator i = b; i != e; ++i)
	{
		InsertPath(*i);
	}

	DWORD endTime = GetTickCount();
	TRACE("PathLayer::Recalc took %d ms\n", endTime - startTime);
}

void PathLayer::InsertPath(const HotspotPath &path)
{
	if (path.size() < 2)
		return;	// Don't bother.

	HotspotPath::const_iterator b = path.begin();
	HotspotPath::const_iterator e = path.end();
	ASSERT(b != e);	// Empty path?

	FILETIME timestamp;
	timestamp.dwHighDateTime = timestamp.dwLowDateTime = 0;

	for (HotspotPath::const_iterator i = b+1; i != e; ++i)
	{
		int prev_id = *(i-1);
		int this_id = *i;

		// Find out where the c.o.g. of that hotspot is.  It'll be in logical coordinates.
		CentreOfGravity c1 = (*m_centres)[prev_id];
		CentreOfGravity c2 = (*m_centres)[this_id];

		CPoint p1 = c1.point;
		CPoint p2 = c2.point;

		COLORREF cr = RGB(255,0,255);
		int width = 3;
		InsertLineDef(p1, p2, cr, width, timestamp, 0);
	}
}
#endif /* defined(ENABLE_DESIGN_MODE) */
