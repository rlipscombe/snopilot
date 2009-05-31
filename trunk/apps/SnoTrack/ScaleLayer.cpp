#include "StdAfx.h"
#include "ScaleLayer.h"
#include "DrawSite.h"
#include "win32_ui/bold_font.h"

void ScaleLayer::OnScroll(CWnd *pWnd, int xAmount, int yAmount)
{
	CRect lastDrawRect = m_lastDrawRect;
	if (lastDrawRect.IsRectEmpty())
		return;

	// Invalidate where it's going to be.
	pWnd->InvalidateRect(lastDrawRect, FALSE);

	// Figure out where the old one went.
	lastDrawRect.OffsetRect(xAmount, yAmount);

	// Invalidate that spot too.
	pWnd->InvalidateRect(lastDrawRect, FALSE);
}

// 5 miles is 8 km.  10km is 6.25mi.  So, we'll use a scale based on 1:2.
// i.e. 2000km / 1250mi (1000mi)
//       200km / 125mi  (100mi)
//        20km / 12.5mi (10mi)
//
// Then we get to metres vs. yards, where the size is roughly the same:
// [5280 feet = 1760 yards = 1 mile]
//       2000m / 2187 yards (2000 yards)
//        200m / 219 yards (200 yards)
//         20m / 22 yards (20 yards)
ScaleLayer::ScaleValues ScaleLayer::metricScaleTable[] = {
	//         20m / 22 yards (20 yards)
	{ 8, 20 * 10, 8, "0", "metres", "20", },
	//        200m / 219 yards (200 yards)
	{ 64, 200 * 10, 8, "0", "metres", "200", },
	//       2000m / 2187 yards (2000 yards)
	{ 512, 2000 * 10, 8, "0", "metres", "2000", },
	//        20km / 12.5mi (10mi)
	{ 8192, 20 * 1000 * 10, 8, "0", "km", "20", },
	//       200km / 125mi  (100mi)
	{ 65536, 200 * 1000 * 10, 8, "0", "km", "200", },
};

ScaleLayer::ScaleValues ScaleLayer::imperialScaleTable[] = {
	//         20m / 22 yards (20 yards)
	{ 8, 183, 8, "0", "yards", "20", }, // 183 = 20 * 36 * 2.54 / 10;
	//        200m / 219 yards (200 yards)
	{ 64, 1829, 8, "0", "yards", "200", },	// 1829 = 200 * 36 * 2.54 / 10;
	//       2000m / 2187 yards (2000 yards)
	{ 512, 18288, 8, "0", "yards", "2000", },	// 18288 = 2000 * 36 * 2.54 / 10;
	//        20km / 12.5mi (10mi)
	{ 8192, 160934, 4, "0", "miles", "10", },	// 160934 = 10 * 1760 * 36 * 2.54 / 10;
	//       200km / 125mi  (100mi)
	{ 65536, 1609344, 4, "0", "miles", "100", },	// 1609344 = 100 * 1760 * 36 * 2.54 / 10;
};

//      2000km / 1250mi (1000mi)
ScaleLayer::ScaleValues ScaleLayer::largeMetricScale = { -1, 2000 * 1000 * 10, 8, "0", "km", "2000", };
ScaleLayer::ScaleValues ScaleLayer::largeImperialScale = { -1, 16093440, 4, "0", "miles", "1000", };	// 16093440 = 1000 * 1760 * 36 * 2.54 / 10;

const ScaleLayer::ScaleValues *ScaleLayer::CalculateMetricScaleValues(int scaleFactor)
{
	for (const ScaleValues *p = metricScaleTable; p->lhsText; ++p)
	{
		if (scaleFactor < p->maxScaleFactor)
		{
			return p;
		}
	}

	return &largeMetricScale;
}

void ScaleLayer::OnDraw(DrawSite *drawSite, CDC *pDC)
{
	int scaleFactor = drawSite->GetScaleFactor();
//	TRACE("scaleFactor = %d\n", scaleFactor);

	CRect clientRect = drawSite->GetClientRect();

	LOGFONT lf;
	GetMessageBoxFont(&lf);
	CFont font;
	VERIFY(font.CreateFontIndirect(&lf));

	CFont *oldFont = pDC->SelectObject(&font);

	CRect drawRect(0,0,0,0);

	const ScaleValues *p = CalculateMetricScaleValues(scaleFactor);
	DrawMetricScale(pDC, scaleFactor, clientRect, p->scaleDecimetres, p->parts, p->lhsText, p->midText, p->rhsText, &drawRect);
//	const ScaleValues *q = CalculateImperialScaleValues(scaleFactor);
//	DrawImperialScale(pDC, scaleFactor, clientRect, q->scaleDecimetres, q->parts, q->lhsText, q->midText, q->rhsText, &drawRect);

	pDC->SelectObject(oldFont);

	// Allow for the fact that DrawPartitionedRectangle draws slightly outside the given rectangle.
	drawRect.InflateRect(1,1,1,1);
	m_lastDrawRect = drawRect;
}

CRect ScaleLayer::CalculateMetricRect(int scaleFactor, const CRect &clientRect, int metricScaleDecimetres)
{
	int metricScalePixels = metricScaleDecimetres / scaleFactor;

	CRect metricRect;

	metricRect.left = clientRect.left + 20;
	metricRect.bottom = clientRect.bottom - 20;
	metricRect.right = metricRect.left + metricScalePixels;
	metricRect.top = metricRect.bottom - 10;

	return metricRect;
}

void ScaleLayer::DrawMetricScale(CDC *pDC, int scaleFactor, const CRect &clientRect, int metricScaleDecimetres, int numPortions,
								 const CString &lhsText, const CString &midText, const CString &rhsText, CRect *lastDrawRect)
{
	CRect metricRect = CalculateMetricRect(scaleFactor, clientRect, metricScaleDecimetres);

	DrawPartitionedRectangle(pDC, metricRect, numPortions);

	// Draw the labels just under the ends:
	DrawScaleTextBelow(pDC, &metricRect, lhsText, midText, rhsText);

	lastDrawRect->UnionRect(lastDrawRect, metricRect);
}

CRect ScaleLayer::CalculateImperialRect(int scaleFactor, const CRect &clientRect, int imperialScaleDecimetres)
{
	int imperialScalePixels = imperialScaleDecimetres / scaleFactor;

	CRect imperialRect;

	imperialRect.left = clientRect.left + 20;
	imperialRect.bottom = clientRect.bottom - 30;
	imperialRect.right = imperialRect.left + imperialScalePixels;
	imperialRect.top = imperialRect.bottom - 10;

	return imperialRect;
}

void ScaleLayer::DrawImperialScale(CDC *pDC, int scaleFactor, const CRect &clientRect, int imperialScaleDecimetres, int numPortions,
								   const CString &lhsText, const CString &midText, const CString &rhsText, CRect *lastDrawRect)
{
	CRect imperialRect = CalculateImperialRect(scaleFactor, clientRect, imperialScaleDecimetres);

	DrawPartitionedRectangle(pDC, imperialRect, numPortions);

	// Draw the labels just above the ends:
	DrawScaleTextAbove(pDC, &imperialRect, lhsText, midText, rhsText);

	lastDrawRect->UnionRect(lastDrawRect, imperialRect);
}

void ScaleLayer::DrawScaleTextBelow(CDC *pDC, CRect *scaleRect, const CString &lhsText, const CString &midText, const CString &rhsText)
{
	pDC->SetTextColor(RGB(0x00,0x00,0x00));
	pDC->SetBkColor(RGB(0xFF,0xFF,0xFF));

	int y = scaleRect->bottom + 3;

	CSize textSize = pDC->GetTextExtent(lhsText);
	int x = scaleRect->left - textSize.cx / 2;
	pDC->ExtTextOut(x, y, 0, NULL, lhsText, NULL);

	CSize textSize2 = pDC->GetTextExtent(midText);
	int x2 = scaleRect->CenterPoint().x - textSize2.cx / 2;
	pDC->ExtTextOut(x2, y, 0, NULL, midText, NULL);

	CSize textSize3 = pDC->GetTextExtent(rhsText);
	int x3 = scaleRect->right - textSize3.cx / 2;
	pDC->ExtTextOut(x3, y, 0, NULL, rhsText, NULL);

	// Bump it a little to allow for the text
	scaleRect->left -= textSize.cx / 2;
	scaleRect->right += textSize3.cx / 2;
	scaleRect->bottom += textSize.cy + 3;
}

void ScaleLayer::DrawScaleTextAbove(CDC *pDC, CRect *scaleRect, const CString &lhsText, const CString &midText, const CString &rhsText)
{
	pDC->SetTextColor(RGB(0x00,0x00,0x00));
	pDC->SetBkColor(RGB(0xFF,0xFF,0xFF));

	CSize textSize = pDC->GetTextExtent(lhsText);
	int x = scaleRect->left - textSize.cx / 2;
	int y = scaleRect->top - textSize.cy - 3;
	pDC->ExtTextOut(x, y, 0, NULL, lhsText, NULL);

	CSize textSize2 = pDC->GetTextExtent(midText);
	int x2 = scaleRect->CenterPoint().x - textSize2.cx / 2;
	int y2 = scaleRect->top - textSize2.cy - 3;
	pDC->ExtTextOut(x2, y, 0, NULL, midText, NULL);

	CSize textSize3 = pDC->GetTextExtent(rhsText);
	int x3 = scaleRect->right - textSize3.cx / 2;
	int y3 = scaleRect->top - textSize3.cy - 3;
	pDC->ExtTextOut(x3, y3, 0, NULL, rhsText, NULL);

	// Bump it a little to allow for the text
	scaleRect->left -= textSize.cx / 2;
	scaleRect->right += textSize3.cx / 2;
	scaleRect->top -= textSize.cy + 3;
}

void ScaleLayer::DrawPartitionedRectangle(CDC *pDC, const CRect &rect, int numPortions)
{
	CPen edgePen(PS_SOLID, 1, RGB(0x00,0x00,0x00));
	CPen *oldPen = pDC->SelectObject(&edgePen);

	CBrush lightBrush(RGB(0xFF,0xFF,0xDD));
	CBrush darkBrush(RGB(0xC0,0xC0,0xC0));

	CBrush *oldBrush = pDC->SelectObject(&lightBrush);
	
	// Divide that into quarters or eighths, depending:
	CRect r(rect);

	int partitionWidth = rect.Width() / numPortions;
	for (int i = 0; i < numPortions; ++i)
	{
		r.right = r.left + partitionWidth;

		if (i % 2)
			pDC->SelectObject(&lightBrush);
		else
			pDC->SelectObject(&darkBrush);

		pDC->Rectangle(r.left-1, r.top-1, r.right, r.bottom);

		r.left = r.right;
	}

	pDC->SelectObject(oldBrush);
	pDC->SelectObject(oldPen);
}