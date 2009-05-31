#pragma once
#include "Layer.h"

class DeviceRangeLayer : public Layer
{
public:
	DeviceRangeLayer(const TCHAR *name, int weight)
		: Layer(name, weight, false, false)
	{
	}

// Layer
public:
	virtual void OnDraw(DrawSite *drawSite, CDC *pDC);
};
