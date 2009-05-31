#pragma once
#include "Layer.h"

class LogicalRangeLayer : public Layer
{
public:
	LogicalRangeLayer(const TCHAR *name, int weight)
		: Layer(name, weight, false, false)
	{
	}

// Layer
public:
	virtual void OnDraw(DrawSite *drawSite, CDC *pDC);
};

