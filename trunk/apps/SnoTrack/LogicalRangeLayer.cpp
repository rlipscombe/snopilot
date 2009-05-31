#include "StdAfx.h"
#include "LogicalRangeLayer.h"
#include "DrawSite.h"

void LogicalRangeLayer::OnDraw(DrawSite *drawSite, CDC *pDC)
{
	CRect logicalRange(drawSite->GetLogicalRange());
	CPen dottedPen(PS_DOT, 1, RGB(0, 0, 192));

	CPen *oldPen = pDC->SelectObject(&dottedPen);

	pDC->MoveTo(drawSite->GetClientPointFromLogicalPoint(logicalRange.left, logicalRange.top));
	pDC->LineTo(drawSite->GetClientPointFromLogicalPoint(logicalRange.right, logicalRange.top));
	pDC->LineTo(drawSite->GetClientPointFromLogicalPoint(logicalRange.right, logicalRange.bottom));
	pDC->LineTo(drawSite->GetClientPointFromLogicalPoint(logicalRange.left, logicalRange.bottom));
	pDC->LineTo(drawSite->GetClientPointFromLogicalPoint(logicalRange.left, logicalRange.top));

	pDC->SelectObject(oldPen);
}

