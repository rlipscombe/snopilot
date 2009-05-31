#pragma once
#include "Layer.h"
#include "Waypoint.h"

class WaypointsLayer : public Layer
{
	const WaypointCollection *m_waypoints;
	COLORREF m_crText;
	COLORREF m_crBack;
	bool m_bShowWaypointPosition;

public:
	WaypointsLayer(const TCHAR *name, int weight, const WaypointCollection *waypoints, COLORREF crText, COLORREF crBack, bool bShowWaypointPosition)
		: Layer(name, weight, true, false), m_waypoints(waypoints), m_crText(crText), m_crBack(crBack), m_bShowWaypointPosition(bShowWaypointPosition)
	{
	}

// Layer
public:
	virtual void OnDraw(DrawSite *drawSite, CDC *pDC);
};
