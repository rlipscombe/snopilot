#include "StdAfx.h"
#include "HotspotVisit.h"
#include "HotspotGrid.h"

void CalculateHotspotVisits(const PositionFixCollection &positions, const HotspotGrid *hotspotGrid, HotspotVisitCollection *visits)
{
	visits->clear();

	int last_id = HotspotGrid::INVALID_HOTSPOT_ID;
	FILETIME last_timestamp;
	last_timestamp.dwHighDateTime = last_timestamp.dwLowDateTime = 0;
	HotspotVisit visit;

	PositionFixCollection::const_iterator b = positions.begin();
	PositionFixCollection::const_iterator e = positions.end();
	for (PositionFixCollection::const_iterator i = b; i != e; ++i)
	{
		CPoint logicalPoint(i->GetProjectionX(), i->GetProjectionY());

		int id;
		if (!hotspotGrid->GetHotspotFromLogicalPoint(logicalPoint, &id))
			id = HotspotGrid::INVALID_HOTSPOT_ID;

		if (id != last_id)
		{
			if (id != HotspotGrid::INVALID_HOTSPOT_ID)
			{
				// We've just entered a hotspot
				visit.id = id;
				visit.entryTime = i->GetTimestamp();
				
//				TRACE("Entered hotspot %d at %d\n", visit.id, visit.entryTime);
			}
			else
			{
				// We've just left a hotspot.
				visit.exitTime = last_timestamp;
				visit.imageIndex = 0;

//				TRACE("Left hotspot %d at %d\n", visit.id, visit.exitTime);
				visits->push_back(visit);
			}

		}

		last_id = id;
		last_timestamp = i->GetTimestamp();
	}

	if (last_id != HotspotGrid::INVALID_HOTSPOT_ID)
	{
		// TODO: We've got an unfinished visit.
		TRACE("Unfinished visit!\n");
	}

	// Go through the hotspots, removing spots where we left and reentered a hotspot:
	if (visits->size() < 2)
		return;

	for (HotspotVisitCollection::iterator i = visits->begin()+1; i != visits->end(); )
	{
		int prev_id = (i-1)->id;
		int this_id = i->id;

		if (prev_id == this_id)
		{
//			TRACE("Folded duplicate visit to hotspot #%d\n", this_id);
			(i-1)->exitTime = i->exitTime;

			i = visits->erase(i);
		}
		else
			++i;
	}
}

