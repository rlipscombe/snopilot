#pragma once
#include "Layer.h"

class HotspotGrid;

class HotspotGridLayer : public Layer
{
	HotspotGrid *m_hotspots;

public:
	HotspotGridLayer(const TCHAR *name, int weight, HotspotGrid *hotspots)
		: Layer(name, weight, false, true), m_hotspots(hotspots)
	{
	}

// Layer
public:
	virtual void OnDraw(DrawSite *drawSite, CDC *pDC);
};

