#include "StdAfx.h"
#include "LiftsLayer.h"
#include "DrawSite.h"
#include "win32_ui/bold_font.h"
#include "rotation.h"

/** For each lift, draw a line from bottom to top, drawing the label about half way up. */
void LiftsLayer::OnDraw(DrawSite *drawSite, CDC *pDC)
{
	CPen liftPen(PS_DASH, 2, RGB(0, 0, 192));
	CPen *oldPen = pDC->SelectObject(&liftPen);

	for (LiftCollection::const_iterator i = m_lifts->begin(); i != m_lifts->end(); ++i)
	{
		CPoint logical_bottom(i->GetBottomProjectionX(), i->GetBottomProjectionY());
		CPoint logical_top(i->GetTopProjectionX(), i->GetTopProjectionY());

		CPoint client_bottom = drawSite->GetClientPointFromLogicalPoint(logical_bottom);
		CPoint client_top = drawSite->GetClientPointFromLogicalPoint(logical_top);

		pDC->MoveTo(client_bottom);
		pDC->LineTo(client_top);

		DrawLiftLabel(pDC, *i, client_bottom, client_top);
	}

	pDC->SelectObject(oldPen);
};

void LiftsLayer::DrawLiftLabel(CDC *pDC, const Lift &lift, CPoint client_bottom, CPoint client_top)
{
	CString liftName = ConvertUTF8ToSystem(lift.GetLabel()).c_str();

	// Use a normal font to work out the size of the text.
	LOGFONT lf;
	GetMessageBoxFont(&lf);
	_tcscpy(lf.lfFaceName, _T("Arial"));

	CFont normalFont;
	VERIFY(normalFont.CreateFontIndirect(&lf));
	CFont *oldFont = pDC->SelectObject(&normalFont);
	pDC->SetTextColor(RGB(0x00,0x00,0xFF));

	CSize textSize = pDC->GetTextExtent(liftName);

	// Now, we can do this using proportions.  If we call the line of the lift AB, then
	// we can create a right-angled triangle ABC.  If we place a point Q, on the line,
	// where we'd like to place the label, and form another right-angled triangle,
	// AQX, we can work out where Q goes using the fact that these triangles
	// are "similar triangles".  We can then work out where the actual point for the text (P)
	// is by similar (heh) methods.
	// See the file LiftsLayerTriangles.png, in the source directory.
	CPoint a(client_top);
	CPoint b(client_bottom);

	if (a.x > b.x)
		std::swap(a, b);

	double bc = b.y - a.y;
	double ac = b.x - a.x;
	double ab = hypot(ac, bc);	//sqrt((ac * ac) + (bc * bc));

	// If the text is shorter than the line, then draw it.
	if (textSize.cx < ab)
	{
		double aq = (ab - textSize.cx) / 2;
		double nq = (aq * bc) / ab;
		double an = (ac * aq) / ab;

		CPoint q(a.x + (int)an, a.y + (int)nq);

		// We then sling another small triangle in there.  This one is PLQ.
		const int textOffset = 5;
		double pq = textSize.cy + textOffset;
		double lp = (ac * pq) / ab;
		double lq = (bc * pq) / ab;

		CPoint p(q.x + (int)lq, q.y - (int)lp);

		// Work out the actual angle of the line: we don't actually need this for positioning, but we _do_
		// need it to fill in the LOGFONT.
		double opp = client_bottom.y - client_top.y;
		double adj = client_top.x - client_bottom.x;
		double theta = atan(opp/adj);

		// Then replace it with a rotated font to actually draw the text.
		lf.lfEscapement = lf.lfOrientation = (int)(10 * RadiansToDegrees(theta));

		CFont rotatedFont;
		VERIFY(rotatedFont.CreateFontIndirect(&lf));
		pDC->SelectObject(&rotatedFont);

		pDC->ExtTextOut(p.x, p.y, 0, NULL, liftName, NULL);
	}

	pDC->SelectObject(oldFont);
}