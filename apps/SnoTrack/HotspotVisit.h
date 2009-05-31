#pragma once
#include <vector>
#include "PositionFix.h"

class HotspotGrid;

struct HotspotVisit
{
	int id;
	FILETIME entryTime;
	FILETIME exitTime;
	int imageIndex;
};

typedef std::vector< HotspotVisit > HotspotVisitCollection;

void CalculateHotspotVisits(const PositionFixCollection &positions, const HotspotGrid *hotspotGrid, HotspotVisitCollection *visits);
