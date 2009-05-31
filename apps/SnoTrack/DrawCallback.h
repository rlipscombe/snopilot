#pragma once
#include "BspNode.h"
#include "win32_ui/PenCache.h"

class DrawSite;

class DrawCallback : public BspSearchCallback
{
	DrawSite *m_drawSite;
	CDC *m_pDC;

	PenCache m_pens;

	int m_linesPlotted;

public:
	DrawCallback(DrawSite *drawSite, CDC *pDC)
		: m_drawSite(drawSite), m_pDC(pDC), m_linesPlotted(0)
	{
	}

	int GetLinesPlotted() const { return m_linesPlotted; }

	virtual void OnLeafNode(const CRect &rect, const items_t &items);
};

