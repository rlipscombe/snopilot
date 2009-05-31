#include "StdAfx.h"
#include "DrawCallback.h"
#include "DrawSite.h"

void DrawCallback::OnLeafNode(const CRect &rect, const items_t &items)
{
	items_t::const_iterator b = items.begin();
	items_t::const_iterator e = items.end();

	CPen *oldPen = NULL;
	CPen *currentPen = NULL;
	for (items_t::const_iterator i = b; i != e; ++i)
	{
		int penWidth = i->m_width;

		CPen *newPen = m_pens.GetPen(PS_SOLID, penWidth, i->m_cr);
		if (newPen != currentPen)
		{
			CPen *temp = m_pDC->SelectObject(newPen);
			if (!oldPen)
				oldPen = temp;

			currentPen = newPen;
		}

		CPoint p1 = m_drawSite->GetClientPointFromLogicalPoint(i->m_x1, i->m_y1);
		CPoint p2 = m_drawSite->GetClientPointFromLogicalPoint(i->m_x2, i->m_y2);

		if (p1 != p2)
		{
			m_pDC->MoveTo(p1);
			m_pDC->LineTo(p2);
		}

		++m_linesPlotted;
	}

	if (oldPen)
		m_pDC->SelectObject(oldPen);
}
