#include "StdAfx.h"
#include "GridLinesLayer.h"
#include "DrawSite.h"
#include "GridBounds.h"
#include "Rotation.h"
#include "LatLon.h"
#include "win32_ui/bold_font.h"

void GridLinesLayer::DrawLongitudeLabels(DrawSite *drawSite, CDC *pDC, double lon, CPoint p0, CPoint p1)
{
	CString label = FormatLongitude(lon, FORMAT_LON_GARMIN);
	CSize textSize = pDC->GetTextExtent(label);

	// The end of the line is at p0.  We need to play with the coordinates to allow for the rotation:
	int yOffset = textSize.cy + 5;
	int xOffset = 5;

	Rotation rotation = drawSite->GetRotation();
	
	CPoint t0(p0);
	t0.x -= (int)(yOffset * rotation.sin_theta);
	t0.x += (int)(xOffset * rotation.cos_theta);

	t0.y -= (int)(yOffset * rotation.cos_theta);
	t0.y -= (int)(xOffset * rotation.sin_theta);

	VERIFY(pDC->ExtTextOut(t0.x, t0.y, 0, NULL, label, NULL));

	CPoint t1(p1);
	t1.x -= (int)(yOffset * rotation.sin_theta);
	t1.x += (int)(xOffset * rotation.cos_theta);

	t1.y -= (int)(yOffset * rotation.cos_theta);
	t1.y -= (int)(xOffset * rotation.sin_theta);

	VERIFY(pDC->ExtTextOut(t1.x, t1.y, 0, NULL, label, NULL));
}

void GridLinesLayer::DrawLatitudeLabels(DrawSite *drawSite, CDC *pDC, double lat, CPoint p0, CPoint p1)
{
	CString label = FormatLatitude(lat, FORMAT_LAT_GARMIN);
	CSize textSize = pDC->GetTextExtent(label);

	// The end of the line is at p0.  We need to play with the coordinates to allow for the rotation:
	int xOffset = textSize.cy + 5;
	int yOffset = 5;

	Rotation rotation = drawSite->GetRotation();
	
	CPoint t0(p0);
	t0.x += (int)(yOffset * rotation.sin_theta);
	t0.x -= (int)(xOffset * rotation.cos_theta);

	t0.y += (int)(yOffset * rotation.cos_theta);
	t0.y += (int)(xOffset * rotation.sin_theta);

	VERIFY(pDC->ExtTextOut(t0.x, t0.y, 0, NULL, label, NULL));

	CPoint t1(p1);
	t1.x += (int)(yOffset * rotation.sin_theta);
	t1.x -= (int)(xOffset * rotation.cos_theta);

	t1.y += (int)(yOffset * rotation.cos_theta);
	t1.y += (int)(xOffset * rotation.sin_theta);

	VERIFY(pDC->ExtTextOut(t1.x, t1.y, 0, NULL, label, NULL));
}

void GridLinesLayer::DrawLongitudeLine(DrawSite *drawSite, CDC *pDC, double lon, double minLat, double maxLat)
{
	// The line runs between (lon,lat0) and (lon,lat1)
	CPoint p0 = drawSite->GetClientPointFromSphere(lon, minLat);
	CPoint p1 = drawSite->GetClientPointFromSphere(lon, maxLat);

	pDC->MoveTo(p0);
	pDC->LineTo(p1);

	DrawLongitudeLabels(drawSite, pDC, lon, p0, p1);
}

void GridLinesLayer::DrawLatitudeLine(DrawSite *drawSite, CDC *pDC, double lat, double minLon, double maxLon)
{
	// The line runs between (lon0,lat) and (lon1,lat)
	CPoint p0 = drawSite->GetClientPointFromSphere(minLon, lat);
	CPoint p1 = drawSite->GetClientPointFromSphere(maxLon, lat);

	pDC->MoveTo(p0);
	pDC->LineTo(p1);

	DrawLatitudeLabels(drawSite, pDC, lat, p0, p1);
}

void GridLinesLayer::DrawLongitudeLines(DrawSite *drawSite, CDC *pDC, GridBounds bounds)
{
	pDC->SetTextColor(RGB(192,192,0));
	pDC->SetBkColor(RGB(255,255,255));

	Rotation rotation = drawSite->GetRotation();
	
	LOGFONT lf;
	GetMessageBoxFont(&lf);
	_tcscpy(lf.lfFaceName, _T("Arial"));
	lf.lfEscapement = lf.lfOrientation = (int)(10 * RadiansToDegrees(rotation.theta));

	CFont font;
	font.CreateFontIndirect(&lf);
	CFont *oldFont = pDC->SelectObject(&font);

	for (double lon = bounds.m_minLongitude; lon <= bounds.m_maxLongitude; lon += GetLongitudeSpacing())
	{
		DrawLongitudeLine(drawSite, pDC, lon, bounds.m_minLatitude, bounds.m_maxLatitude);
	}

	pDC->SelectObject(oldFont);
}

void GridLinesLayer::DrawLatitudeLines(DrawSite *drawSite, CDC *pDC, GridBounds bounds)
{
	pDC->SetTextColor(RGB(192,192,0));
	pDC->SetBkColor(RGB(255,255,255));

	Rotation rotation = drawSite->GetRotation();
	
	LOGFONT lf;
	GetMessageBoxFont(&lf);
	_tcscpy(lf.lfFaceName, _T("Arial"));
	lf.lfEscapement = lf.lfOrientation = 900 + (int)(10 * RadiansToDegrees(rotation.theta));

	CFont font;
	font.CreateFontIndirect(&lf);
	CFont *oldFont = pDC->SelectObject(&font);

	for (double lat = bounds.m_minLatitude; lat <= bounds.m_maxLatitude; lat += GetLongitudeSpacing())
	{
		DrawLatitudeLine(drawSite, pDC, lat, bounds.m_minLongitude, bounds.m_maxLongitude);
	}

	pDC->SelectObject(oldFont);
}

void GridLinesLayer::OnDraw(DrawSite *drawSite, CDC *pDC)
{
	/// @todo Figure out a sensible spacing if the scale's that large.
	if (drawSite->GetScaleFactor() > 1024)
		return;

	GridBounds bounds = drawSite->GetBounds().Expand(1.0/60.0, 1.0/60.0);

	CPen gridPen(PS_DOT, 1, RGB(192,192,0));
	CPen *oldPen = pDC->SelectObject(&gridPen);

	DrawLatitudeLines(drawSite, pDC, bounds);
	DrawLongitudeLines(drawSite, pDC, bounds);

	pDC->SelectObject(oldPen);
}

