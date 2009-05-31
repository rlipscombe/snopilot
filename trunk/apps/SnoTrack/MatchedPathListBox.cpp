// MatchedPathListBox.cpp : implementation file
//

#include "stdafx.h"
#include "SnoTrack.h"
#include "MatchedPathListBox.h"
#include "win32_ui/center_rectangle.h"
#include "win32_ui/bold_font.h"
#include "win32_ui/draw_selection.h"
#include "win32/TimeZoneInformation.h"
#include "win32/filetime_helpers.h"

// CMatchedPathListBox

IMPLEMENT_DYNAMIC(CMatchedPathListBox, COwnerDrawListBox)

CMatchedPathListBox::CMatchedPathListBox()
	: m_pImageList(NULL)
{
	memset(&m_tzi, 0, sizeof(TIME_ZONE_INFORMATION));
}

CMatchedPathListBox::~CMatchedPathListBox()
{
}

CImageList *CMatchedPathListBox::SetImageList(CImageList *piml)
{
	CImageList *old = m_pImageList;
	m_pImageList = piml;
	return old;
}

BEGIN_MESSAGE_MAP(CMatchedPathListBox, COwnerDrawListBox)
	ON_WM_SIZE()
END_MESSAGE_MAP()

void CMatchedPathListBox::AddEvent(const CString &description, FILETIME startTime, FILETIME endTime, double distance, int imageIndex)
{
	MatchEvent *event = new MatchEvent;
	event->description = description;
	event->startTime = startTime;
	event->endTime = endTime;
	event->distance = distance;
	event->imageIndex = imageIndex;

	SendMessage(LB_ADDSTRING, 0, (LPARAM)event);
}

bool CMatchedPathListBox::GetEvent(int nIndex, FILETIME *startTime, FILETIME *endTime)
{
	const MatchEvent *event = reinterpret_cast<const MatchEvent *>(GetItemDataPtr(nIndex));
	if (event)
	{
		*startTime = event->startTime;
		*endTime = event->endTime;
		return true;
	}

	return false;
}

CString FormatTimeSpanText(FILETIME startTime, FILETIME endTime, const TIME_ZONE_INFORMATION *tzi)
{
	CString str;
	str.Format("%s -- %s", LPCTSTR(FormatFileTimeMs(startTime, tzi)), LPCTSTR(FormatFileTimeMs(endTime, tzi)));

	return str;
}

void CMatchedPathListBox::DoDrawItem(CDC *pDC, const void *itemData, const CRect &itemRect, DWORD itemState)
{
	const MatchEvent *event = reinterpret_cast<const MatchEvent *>(itemData);
	CString titleText = event->description;
	
	CString timeText = FormatTimeSpanText(event->startTime, event->endTime, &m_tzi);

	CString elapsedText;
	if (event->distance)
	{
		CString elapsed = FormatElapsedFileTimeMs(event->startTime, event->endTime);

		double speed_kmh = event->distance / FileTimeDifferenceInSeconds(event->endTime, event->startTime);
		speed_kmh *= 3600;

		elapsedText.Format("%s (%3.1f km): %3.1f km/h", LPCTSTR(elapsed), event->distance, speed_kmh);
	}
	else
		elapsedText.Format("%s", LPCTSTR(FormatElapsedFileTimeMs(event->startTime, event->endTime)));

	ASSERT(m_font.GetSafeHandle());
	CFont *oldFont = pDC->SelectObject(&m_font);

	CRect innerRect(itemRect);
	innerRect.DeflateRect(16+4+2, 2, 1, 2);

	CRect titleRect(innerRect);		// Run name goes here
	CRect timeRect(innerRect);		// Enter/Exit times go here
	CRect elapsedRect(titleRect);	// Elapsed times go here
	
	titleRect.bottom = titleRect.top + innerRect.Height() / 3;
	timeRect.top = titleRect.bottom;
	timeRect.bottom = timeRect.top + innerRect.Height() / 3;
	elapsedRect.top = timeRect.bottom;
	elapsedRect.bottom = elapsedRect.top + innerRect.Height() / 3;

	pDC->FillSolidRect(itemRect, GetSysColor(COLOR_WINDOW));

	CRect clipRect(itemRect);
	clipRect.DeflateRect(1, 1);

	VERIFY(pDC->ExtTextOut(titleRect.left, titleRect.top, ETO_CLIPPED, clipRect, titleText, NULL));
	VERIFY(pDC->ExtTextOut(timeRect.left, timeRect.top, ETO_CLIPPED, clipRect, timeText, NULL));
	VERIFY(pDC->ExtTextOut(elapsedRect.left, elapsedRect.top, ETO_CLIPPED, clipRect, elapsedText, NULL));

	CPoint ptIcon(itemRect.TopLeft());
	ptIcon.Offset(4, 4);
	if (m_pImageList)
	{
		m_pImageList->Draw(pDC, event->imageIndex, ptIcon, ILD_NORMAL);
	}

	if (itemState & ODS_SELECTED)
	{
		CRect rect(itemRect);
		rect.DeflateRect(1, 1);
		DrawSelection(pDC, rect);
	}
	else
	{
		CRect rect(itemRect);
		rect.DeflateRect(1, 1);

		pDC->FillSolidRect(rect.left, rect.top, rect.Width(), 1, RGB(0xDD,0xDD,0xEE));
	}

	pDC->SelectObject(oldFont);
}

void CMatchedPathListBox::MeasureItem(LPMEASUREITEMSTRUCT measureItem)
{
	CWindowDC dc(NULL);

	TEXTMETRIC tm;

	ASSERT(m_font.GetSafeHandle());
	CFont *oldFont = dc.SelectObject(&m_font);
	dc.GetTextMetrics(&tm);
	dc.SelectObject(oldFont);

	measureItem->itemHeight = 4 + (tm.tmHeight + tm.tmExternalLeading) * 3;
	measureItem->itemWidth = 64;
}

int CMatchedPathListBox::CompareItem(LPCOMPAREITEMSTRUCT compareItems)
{
	const MatchEvent *event1 = reinterpret_cast<const MatchEvent *>(compareItems->itemData1);
	const MatchEvent *event2 = reinterpret_cast<const MatchEvent *>(compareItems->itemData2);

	int result = CompareFileTime(&event1->startTime, &event2->startTime);
//	TRACE("event1: '%s', event2: '%s', result: %d\n", LPCTSTR(event1->description), LPCTSTR(event2->description), result);
	return result;
}

void CMatchedPathListBox::DeleteItem(LPDELETEITEMSTRUCT deleteItem)
{
	MatchEvent *event = reinterpret_cast<MatchEvent *>(deleteItem->itemData);
	delete event;
}

void CMatchedPathListBox::PreSubclassWindow()
{
	// We need to create the font first, because the call to the base class will
	// result in the call to MeasureItem, which is where we need the font.
	LOGFONT lf;
	GetMessageBoxFont(&lf);

	VERIFY(m_font.CreateFontIndirect(&lf));

	COwnerDrawListBox::PreSubclassWindow();
}

void CMatchedPathListBox::OnSize(UINT nType, int cx, int cy)
{
	Invalidate();

	COwnerDrawListBox::OnSize(nType, cx, cy);
}
