#include "StdAfx.h"
#include "DeviceRangeLayer.h"
#include "DrawSite.h"

void DeviceRangeLayer::OnDraw(DrawSite *drawSite, CDC *pDC)
{
	CRect deviceRange(drawSite->GetDeviceRange());
	CPen dashedPen(PS_DASH, 1, RGB(192, 0, 0));
	CPen *oldPen = pDC->SelectObject(&dashedPen);

	pDC->MoveTo(drawSite->GetClientPointFromDevicePoint(deviceRange.left, deviceRange.top));
	pDC->LineTo(drawSite->GetClientPointFromDevicePoint(deviceRange.right, deviceRange.top));
	pDC->LineTo(drawSite->GetClientPointFromDevicePoint(deviceRange.right, deviceRange.bottom));
	pDC->LineTo(drawSite->GetClientPointFromDevicePoint(deviceRange.left, deviceRange.bottom));
	pDC->LineTo(drawSite->GetClientPointFromDevicePoint(deviceRange.left, deviceRange.top));

	pDC->SelectObject(oldPen);
}

