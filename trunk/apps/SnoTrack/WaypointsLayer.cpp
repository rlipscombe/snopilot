#include "StdAfx.h"
#include "WaypointsLayer.h"
#include "DrawSite.h"
#include "win32_ui/bold_font.h"
#include "LatLon.h"

void WaypointsLayer::OnDraw(DrawSite *drawSite, CDC *pDC)
{
	LOGFONT lf;
	GetMessageBoxFont(&lf);

	CFont font;
	font.CreateFontIndirect(&lf);
	CFont *oldFont = pDC->SelectObject(&font);

	for (WaypointCollection::const_iterator i = m_waypoints->begin(); i != m_waypoints->end(); ++i)
	{
		CPoint logical(i->GetProjectionX(), i->GetProjectionY());
		CPoint device = drawSite->GetClientPointFromLogicalPoint(logical);

		// Draw a square at that location
		CRect waypoint(device.x, device.y, device.x, device.y);
		waypoint.InflateRect(5, 5);
		pDC->FillSolidRect(waypoint, m_crText);
		waypoint.DeflateRect(1, 1);
		pDC->FillSolidRect(waypoint, m_crBack);

		// Draw a text label there, too.
		CString label = ConvertUTF8ToSystem(i->GetLabel()).c_str();

		if (m_bShowWaypointPosition)
		{
			CString lon = FormatLongitude(i->GetLongitude(), FORMAT_LON_GARMIN);
			CString lat = FormatLatitude(i->GetLatitude(), FORMAT_LAT_GARMIN);

			CString label_suffix;
			label_suffix.Format(" (%s %s)", (LPCTSTR)lon, (LPCTSTR)lat);

			label += label_suffix;
		}

		CSize textSize = pDC->GetTextExtent(label);
		CRect textRect(waypoint.right + 6, waypoint.top, waypoint.right + 6 + textSize.cx, waypoint.top + textSize.cy);
		pDC->SetBkColor(m_crBack);
		pDC->SetTextColor(m_crText);
		pDC->ExtTextOut(textRect.left, textRect.top, ETO_CLIPPED, textRect, label, NULL);
	}

	pDC->SelectObject(oldFont);
}
