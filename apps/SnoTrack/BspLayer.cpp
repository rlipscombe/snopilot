#include "StdAfx.h"
#include "BspLayer.h"
#include "DrawCallback.h"
#include "DrawSite.h"

BspLayer::~BspLayer()
{
	delete m_bsp;
}

void BspLayer::CreateBsp(const CRect &logicalRange)
{
	m_bsp = BspNode::Create(logicalRange, 12);
}

void BspLayer::DestroyBsp()
{
	delete m_bsp;
	m_bsp = NULL;
}

void BspLayer::InsertLineDef(const LineDef &line)
{
	m_bsp->Insert(line);
}

void BspLayer::InsertLineDef(int x1, int y1, int x2, int y2, COLORREF cr, int width, FILETIME timestamp, ULONGLONG gap)
{
	LineDef line(x1, y1, x2, y2, cr, width, timestamp, gap);
	InsertLineDef(line);
}

void BspLayer::InsertLineDef(CPoint p1, CPoint p2, COLORREF cr, int width, FILETIME timestamp, ULONGLONG gap)
{
	InsertLineDef(p1.x, p1.y, p2.x, p2.y, cr, width, timestamp, gap);
}

void BspLayer::OnDraw(DrawSite *drawSite, CDC *pDC)
{
	if (!m_bsp)
		return;

	BspNode::Rectangle cull = GetCullingRegion(drawSite);

	DWORD startTime = GetTickCount();

	DrawCallback drawCallback(drawSite, pDC);
	m_bsp->Search(cull, &drawCallback);

	DWORD endTime = GetTickCount();
	TRACE("Plotting %d lines took %d ms.\n", drawCallback.GetLinesPlotted(), endTime - startTime);
}

/** To optimise the drawing, we use a BSP containing all of the tracklog points.
 * This effectively gives us a bunch of squares.
 * We cull the draw list by only drawing those squares that overlap with the
 * invalid region of the window.
 * So we need to map the invalid rect into logical coordinates.
 */
BspNode::Rectangle BspLayer::GetCullingRegion(DrawSite *drawSite)
{
	CRect rect = drawSite->GetInvalidRect();

	CPoint a0(rect.left, rect.bottom);
	CPoint b0(rect.right, rect.bottom);
	CPoint c0(rect.right, rect.top);
	CPoint d0(rect.left, rect.top);

	CPoint a1(drawSite->GetLogicalPointFromClientPoint(a0));
	CPoint b1(drawSite->GetLogicalPointFromClientPoint(b0));
	CPoint c1(drawSite->GetLogicalPointFromClientPoint(c0));
	CPoint d1(drawSite->GetLogicalPointFromClientPoint(d0));

	BspNode::Rectangle view(a1, b1, c1, d1);
	return view;
}
