#include "StdAfx.h"
#include "HotspotGridLayer.h"
#include "HotspotGrid.h"
#include "DrawSite.h"
#include "win32_ui/bold_font.h"

class HighlightHotspots : public HotspotGridCallback
{
	DrawSite *m_drawSite;
	CDC *m_pDC;

	CPen m_pen;
	CFont m_font;

public:
	HighlightHotspots(DrawSite *drawSite, CDC *pDC)
		: m_drawSite(drawSite), m_pDC(pDC), m_pen(PS_SOLID, 1, RGB(255,0,0))
	{
		LOGFONT lf;
		GetMessageBoxFont(&lf);

		VERIFY(m_font.CreateFontIndirect(&lf));
	}

	virtual void OnHotspot(int xCell, int yCell, int hotspotId, CRect logicalRect)
	{
		// Figure that out -- we need to highlight that rectangle.
		CPoint a(m_drawSite->GetClientPointFromLogicalPoint(logicalRect.left, logicalRect.top));
		CPoint b(m_drawSite->GetClientPointFromLogicalPoint(logicalRect.right, logicalRect.top));
		CPoint c(m_drawSite->GetClientPointFromLogicalPoint(logicalRect.right, logicalRect.bottom));
		CPoint d(m_drawSite->GetClientPointFromLogicalPoint(logicalRect.left, logicalRect.bottom));

		CPen *oldPen = m_pDC->SelectObject(&m_pen);

		m_pDC->MoveTo(a);
		m_pDC->LineTo(b);
		m_pDC->LineTo(c);
		m_pDC->LineTo(d);
		m_pDC->LineTo(a);

		m_pDC->SelectObject(oldPen);

		// ...and draw the hotspot ID in the center of the cell.
		if (m_drawSite->GetScaleFactor() <= 64)
		{
			CFont *oldFont = m_pDC->SelectObject(&m_font);

			CString str;
			str.Format("%d", hotspotId);

			CSize textSize = m_pDC->GetTextExtent(str);

			int x = (a.x + b.x + c.x + d.x) / 4;
			int y = (a.y + b.y + c.y + d.y) / 4;

			x -= textSize.cx / 2;
			y -= textSize.cy / 2;

			m_pDC->SetTextColor(RGB(0,0,255));
			m_pDC->ExtTextOut(x, y, 0, NULL, str, NULL);

			m_pDC->SelectObject(oldFont);
		}
	}
};

void HotspotGridLayer::OnDraw(DrawSite *drawSite, CDC *pDC)
{
	DWORD startTime = GetTickCount();

	CRect hotspotRange = m_hotspots->GetRange();

	// Draw a bunch of horizontal lines.
	CPen lightPen(PS_SOLID, 1, RGB(0xFF,0xFF,0x00));
	CPen darkPen(PS_SOLID, 1, RGB(0x80,0x80,0x00));
	CPen *oldPen = pDC->SelectObject(&lightPen);

	const int cellSpacing = m_hotspots->GetCellSpacing();

	// Some magic numbers that control when we change pens.
	// We use a dark pen for every tenth line.
	const int tenCells = cellSpacing * 10;

	// We change to a light pen when we move right to the 11th line...
	const int selectLight1 = cellSpacing;
	// ...or, when we're in the negative quadrant, we move right from the -10th line:
	const int selectLight2 = -9 * cellSpacing;

	for (int x = hotspotRange.left; x <= hotspotRange.right; x += cellSpacing)
	{
		if ((x % tenCells) == 0)
			pDC->SelectObject(&darkPen);
		else if (((x % tenCells) == selectLight1) || ((x % tenCells) == selectLight2))
			pDC->SelectObject(&lightPen);

		pDC->MoveTo(drawSite->GetClientPointFromLogicalPoint(x, hotspotRange.top));
		pDC->LineTo(drawSite->GetClientPointFromLogicalPoint(x, hotspotRange.bottom));
	}

	for (int y = hotspotRange.top; y <= hotspotRange.bottom; y += cellSpacing)
	{
		if ((y % tenCells) == 0)
			pDC->SelectObject(&darkPen);
		else if (((y % tenCells) == selectLight1) || ((y % tenCells) == selectLight2))
			pDC->SelectObject(&lightPen);

		pDC->MoveTo(drawSite->GetClientPointFromLogicalPoint(hotspotRange.left, y));
		pDC->LineTo(drawSite->GetClientPointFromLogicalPoint(hotspotRange.right, y));
	}

	// OK, now what we really want here is a list of the hotspot cells that are visible, so that we
	// can draw them nicely.  Knowing which ones are visible is just an optimisation at this point, though,
	// so we'll just enumerate them:
	HighlightHotspots cb(drawSite, pDC);
	m_hotspots->EnumerateHotspots(&cb);

	pDC->SelectObject(oldPen);

	DWORD endTime = GetTickCount();

	TRACE("Drawing the hotspot grid layer took %d ms\n", endTime - startTime);
}
