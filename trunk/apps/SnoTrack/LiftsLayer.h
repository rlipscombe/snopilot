#pragma once
#include "Layer.h"
#include "tstring.h"
#include "Lift.h"

class LiftsLayer : public Layer
{
	const LiftCollection *m_lifts;

public:
	LiftsLayer(const TCHAR *name, int weight, const LiftCollection *lifts)
		: Layer(name, weight, true, false), m_lifts(lifts)
	{
	}

// Layer
public:
	virtual void OnDraw(DrawSite *drawSite, CDC *pDC);

private:
	void DrawLiftLabel(CDC *pDC, const Lift &lift, CPoint client_bottom, CPoint client_top);
};
