#pragma once
#include "Layer.h"
#include "BspNode.h"

class BspLayer : public Layer
{
	BspNode *m_bsp;

public:
	BspLayer(const TCHAR *name, int weight, bool isEnabled, bool isSpecial)
		: Layer(name, weight, isEnabled, isSpecial), m_bsp(NULL)
	{
	}

	~BspLayer();

public:
	virtual void OnDraw(DrawSite *drawSite, CDC *pDC);

protected:
	void CreateBsp(const CRect &logicalRange);
	void DestroyBsp();

	void WalkBsp(BspWalkCallback *pCallback) { m_bsp->Walk(pCallback); }

	// Different ways to accomplish the same thing.
	void InsertLineDef(const LineDef &line);
	void InsertLineDef(CPoint p1, CPoint p2, COLORREF cr, int width, FILETIME timestamp, ULONGLONG gap);
	void InsertLineDef(int x1, int y1, int x2, int y2, COLORREF cr, int width, FILETIME timestamp, ULONGLONG gap);

private:
	BspNode::Rectangle GetCullingRegion(DrawSite *drawSite);
};
