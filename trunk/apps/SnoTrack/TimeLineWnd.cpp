// TimeLineWnd.cpp : implementation file
//

#include "stdafx.h"
#include "TimeLineWnd.h"
#include "win32/TimeZoneInformation.h"
#include "win32/filetime_helpers.h"
#include "win32_ui/bold_font.h"
#include <algorithm>
#include "CalcRoute.h"
#include "win32_ui/center_rectangle.h"
#include "win32_ui/draw_selection.h"
#include "resource.h"
#include "ChildFrm.h"
#include "CartesianView.h"

// CTimeLineWnd
IMPLEMENT_DYNAMIC(CTimeLineWnd, CWnd)

#define LEFT_MARGIN_COLOUR		RGB(0xDD,0xDD,0xFF)
#define VISIBLE_SPAN_COLOUR		RGB(0xDD,0xDD,0xDD)
#define HIDDEN_SPAN_COLOUR		RGB(0x00,0x00,0xFF)
#define RIGHT_MARGIN_COLOUR		RGB(0xDD,0xDD,0xFF)
#define VISIBLE_SCALE_COLOUR	RGB(0xFF,0xFF,0xFF)
#define FOOTER_COLOUR			RGB(0xFF,0xFF,0xFF)

CTimeLineWnd::CTimeLineWnd()
	: m_positionFixes(NULL), m_zoomIndex(DEFAULT_ZOOM_INDEX),
		m_maxElevation(0.0), m_rangeElevation(0.0),
		m_lastMousePosition(-1, -1), m_bTrackLeave(FALSE),
		m_lastHitTest(-1), m_selectionRect(0,0,0,0)
{
	memset(&m_tzi, 0, sizeof(TIME_ZONE_INFORMATION));
}

BOOL CTimeLineWnd::Create(CWnd *pParentWnd, UINT nID)
{
	UINT nClassStyle = 0;
	HCURSOR hCursor = ::LoadCursor(NULL, IDC_ARROW);
	HBRUSH hbrBackground = NULL;	// No need for a brush -- we do it all ourselves, anyway.
	HICON hIcon = NULL;

	LPCTSTR lpszClassName = AfxRegisterWndClass(nClassStyle, hCursor, hbrBackground, hIcon);
	return CWnd::CreateEx(WS_EX_CLIENTEDGE, lpszClassName, NULL, WS_CHILD | WS_VISIBLE | WS_HSCROLL, CRect(0,0,0,0), pParentWnd, nID, NULL);
}

void CTimeLineWnd::UpdateTimeLine(LPCTSTR lpszTimeZoneName, FILETIME minTimestamp, FILETIME maxTimestamp,
								  const PositionFixCollection *positionFixes, double minElevation, double maxElevation,
								  const RouteEventCollection &route)
{
	m_positionFixes = positionFixes;
	GetRegistryTimeZoneInformation(lpszTimeZoneName, &m_tzi);

	ASSERT(maxElevation >= minElevation);

	m_maxElevation = maxElevation;
	m_rangeElevation = maxElevation - minElevation;

	// TODO: Avoid taking a copy of it?
	m_route = route;
	
	RecalcSpans();
	UpdateScrollBar(TRUE);

	int nMin, nMax;
	GetScrollRange(SB_HORZ, &nMin, &nMax);

	// Set these to the same value to signify that there's no selection.
	m_selectionAnchor = minTimestamp;
	m_selectionPoint = minTimestamp;

	ScrollToPos(nMin, 0, TRUE);
	Invalidate();
}

/** How many seconds per pixel? */
int CTimeLineWnd::m_zoomLevels[] = { 1, 5, 15, 30, 60, 120, 240, 360, 720, };
int CTimeLineWnd::m_zoomLevelCount = COUNTOF(CTimeLineWnd::m_zoomLevels);

void CTimeLineWnd::SetZoomIndex(int zoomIndex)
{
	ASSERT(zoomIndex >= 0 && zoomIndex < m_zoomLevelCount);

//	int timestamp = GetRelativeTimeStampFromCenter();

	m_zoomIndex = zoomIndex;

	UpdateScrollBar(FALSE);
	UpdateSelectionRect();
//	CenterOnRelativeTimeStamp(timestamp);
	Invalidate();
}

int CTimeLineWnd::ConvertSecondsToPixels(ULONGLONG seconds) const
{
	int factor = m_zoomLevels[m_zoomIndex];

	ASSERT(seconds >= 0 && seconds <= INT_MAX);
	return (int)(seconds / factor);
}

ULONGLONG CTimeLineWnd::ConvertPixelsToSeconds(int pixels) const
{
	int factor = m_zoomLevels[m_zoomIndex];

	ASSERT(pixels >= 0 && pixels <= INT_MAX);
	return (pixels * factor);
}

int CTimeLineWnd::GetTickIntervalInSeconds() const
{
	int factor = m_zoomLevels[m_zoomIndex];
	return (factor * 60);
}

bool CTimeLineWnd::CanZoomIn() const
{
	if (m_zoomIndex > 0)
		return true;

	return false;
}

bool CTimeLineWnd::CanZoomOut() const
{
	if (m_zoomIndex < m_zoomLevelCount-1)
		return true;

	return false;
}

void CTimeLineWnd::ZoomIn()
{
	if (!CanZoomIn())
		return;

	SetZoomIndex(m_zoomIndex-1);
}

void CTimeLineWnd::ZoomOut()
{
	if (!CanZoomOut())
		return;

	SetZoomIndex(m_zoomIndex+1);
}

// How long must there be no activity before we'll close the span?
const int TIMELINE_INACTIVITY_PERIOD = (60 * 60);

/** Iterate over the position fixes, breaking it into spans.
 * This is used to spot periods of inactivity (e.g. night-time).
 */
void CTimeLineWnd::RecalcSpans()
{
	m_timeSpans.clear();
	
	if (m_positionFixes->empty())
		return;

	// The first span is visible: the first (and every) item in the vector
	// is an item of activity.

	// What we're looking for is a gap between the current item and the previous.
	// If that gap is more than a certain size, then we've found a period of inactivity.

	// Spans are endpoint-exclusive.  This means that the start timestamp
	// is included in the span; the end timestamp isn't.  Practically speaking,
	// this means that the end timestamp of a span is the same as the start
	// timestamp of the following span.

	PositionFixCollection::const_iterator b = m_positionFixes->begin();
	PositionFixCollection::const_iterator e = m_positionFixes->end();

	FILETIME lastTimestamp = b->GetTimestamp();
	FILETIME openTimestamp = b->GetTimestamp();

	for (PositionFixCollection::const_iterator i = b+1; i != e; ++i)
	{
		// We're currently building a visible span.  If it's been more than N seconds since the last timestamp,
		// then we need to close the visible span at that last activity stamp (plus a bit).  We need to create a hidden
		// span starting at that last activity stamp (plus a bit), finishing at the current timestamp (not plus a bit).
		// That leaves us building a visible span, so there's no 'state' flag needed.
		FILETIME thisTimestamp = i->GetTimestamp();

		ULONGLONG diff = FileTimeDifferenceInSeconds(thisTimestamp, lastTimestamp);
		ASSERT(diff >= 0);

		if (diff >= TIMELINE_INACTIVITY_PERIOD)
		{
//			TRACE("Found %I64u seconds of inactivity from %s to %s\n", diff,
//				LPCTSTR(FormatUtcFileTimeInTzSpecificLocalTime(lastTimestamp, &m_tzi)),
//				LPCTSTR(FormatUtcFileTimeInTzSpecificLocalTime(thisTimestamp, &m_tzi)));

			// We have to close the current visible span, create a hidden
			// span, and open a new visible span:
			lastTimestamp += FileTimeFromSeconds(1);

//			TRACE("  Visible span: %s to %s (%I64u seconds)\n",
//				LPCTSTR(FormatUtcFileTimeInTzSpecificLocalTime(openTimestamp, &m_tzi)),
//				LPCTSTR(FormatUtcFileTimeInTzSpecificLocalTime(lastTimestamp, &m_tzi)),
//				FileTimeDifferenceInSeconds(lastTimestamp, openTimestamp));
			m_timeSpans.push_back(TimeSpan(openTimestamp, lastTimestamp, true));

//			TRACE("  Hidden span:  %s to %s (%I64u seconds)\n",
//				LPCTSTR(FormatUtcFileTimeInTzSpecificLocalTime(lastTimestamp, &m_tzi)),
//				LPCTSTR(FormatUtcFileTimeInTzSpecificLocalTime(thisTimestamp, &m_tzi)),
//				FileTimeDifferenceInSeconds(thisTimestamp, lastTimestamp));
			m_timeSpans.push_back(TimeSpan(lastTimestamp, thisTimestamp, false));

//			TRACE("  Visible span: %s to...\n",
//				LPCTSTR(FormatUtcFileTimeInTzSpecificLocalTime(thisTimestamp, &m_tzi)));

			openTimestamp = thisTimestamp;
		}

		lastTimestamp = thisTimestamp;
	}

	// If we've still got an open timestamp, we'd better close it:
	if (lastTimestamp > openTimestamp)
	{
		lastTimestamp += FileTimeFromSeconds(1);
		m_timeSpans.push_back(TimeSpan(openTimestamp, lastTimestamp, true));
	}

//	DumpTimeSpans();
}

void CTimeLineWnd::DumpTimeSpans() const
{
	for (TimeSpanCollection::const_iterator i = m_timeSpans.begin(); i != m_timeSpans.end(); ++i)
	{
		TRACE("%s span from %s to %s (%I64u seconds)\n",
			i->IsVisible() ? "Visible" : "Hidden",
			LPCTSTR(FormatUtcFileTimeInTzSpecificLocalTime(i->GetStartTime(), &m_tzi)),
			LPCTSTR(FormatUtcFileTimeInTzSpecificLocalTime(i->GetEndTime(), &m_tzi)),
			FileTimeDifferenceInSeconds(i->GetEndTime(), i->GetStartTime()));
	}
}

BEGIN_MESSAGE_MAP(CTimeLineWnd, CWnd)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_HSCROLL()
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_MOUSEMOVE()
    ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
    ON_WM_EXITMENULOOP()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()

int CTimeLineWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	LOGFONT lf;
	GetMessageBoxFont(&lf);
	VERIFY(m_scaleFont.CreateFontIndirect(&lf));

	m_scaleHeight = CalculateScaleHeight();
	m_footerHeight = max(18, m_scaleHeight);	// for now

	SetScrollRange(SB_HORZ, 0, 1024, FALSE);
	VERIFY(m_imageList.Create(IDB_EVENT_ICONS, 16, 1, RGB(255,0,255)));

	return 0;
}

const int TICKMARK_HEIGHT = 4;

int CTimeLineWnd::CalculateScaleHeight()
{
	ASSERT(m_scaleFont.GetSafeHandle());

	CWindowDC dc(NULL);
	CFont *oldFont = dc.SelectObject(&m_scaleFont);

	TEXTMETRIC tm;
	VERIFY(dc.GetTextMetrics(&tm));

	dc.SelectObject(oldFont);

	int textHeight = tm.tmExternalLeading + tm.tmHeight;
	return textHeight + TICKMARK_HEIGHT;
}

void CTimeLineWnd::OnSize(UINT nType, int cx, int cy)
{
	if (nType != SIZE_MINIMIZED)
		UpdateScrollBar(TRUE);

	CWnd::OnSize(nType, cx, cy);
}

// Scroll positions are in device (i.e. screen) coordinates:
const int LINE_SIZE = 10;
const int PAGE_SIZE = 100;

void CTimeLineWnd::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if (pScrollBar != NULL && pScrollBar->SendChildNotifyLastMsg())
		return;     // eat it

	switch (nSBCode)
	{
	case SB_LINELEFT:
		ScrollBy(-LINE_SIZE, 0, true);
		break;

	case SB_LINERIGHT:
		ScrollBy(LINE_SIZE, 0, true);
		break;

	case SB_PAGELEFT:
		ScrollBy(-PAGE_SIZE, 0, true);
		break;

	case SB_PAGERIGHT:
		ScrollBy(PAGE_SIZE, 0, true);
		break;

	case SB_LEFT:
		{
			int minPos, maxPos;
			GetScrollRange(SB_HORZ, &minPos, &maxPos);
			SetScrollPos(SB_HORZ, minPos);
		}
		break;

	case SB_RIGHT:
		{
			int minPos, maxPos;
			GetScrollRange(SB_HORZ, &minPos, &maxPos);
			SetScrollPos(SB_HORZ, maxPos);
		}
		break;

	case SB_THUMBTRACK:
		ScrollToThumbTrackPos(true);
		break;
	}
}

void CTimeLineWnd::UpdateScrollBar(BOOL bRedraw)
{
	// How wide is our virtual canvas?
	int widthInPixels = 0;

	TimeSpanCollection::const_iterator b = m_timeSpans.begin();
	TimeSpanCollection::const_iterator e = m_timeSpans.end();

	for (TimeSpanCollection::const_iterator i = b; i != e; ++i)
		widthInPixels += CalculateSpanWidthInPixels(*i);

	// To avoid scrolling past the end of the view, we'll adjust the range to allow for the client area.
	CRect clientRect;
	GetClientRect(&clientRect);
	widthInPixels -= clientRect.Width();

	// And then, we'll allow a _little_ bit of scrolling past the end by setting the scroll range a little wider.
	SetScrollRange(SB_HORZ, -SCROLL_MARGIN, widthInPixels + SCROLL_MARGIN, bRedraw);
}

CPoint CTimeLineWnd::GetScrollPos()
{
	return CPoint(GetScrollPos(SB_HORZ), GetScrollPos(SB_VERT));
}

int CTimeLineWnd::GetTrackPos(int nBar)
{
	SCROLLINFO si;
	memset(&si, 0, sizeof(SCROLLINFO));
	si.cbSize = sizeof(SCROLLINFO);
	GetScrollInfo(nBar, &si, SIF_TRACKPOS);

	return si.nTrackPos;
}

void CTimeLineWnd::ScrollBy(int xScroll, int yScroll, bool bScrollWindow)
{
	CPoint oldPos(GetScrollPos());
	ScrollToPos(oldPos.x + xScroll, oldPos.y + yScroll, bScrollWindow);
}

void CTimeLineWnd::ScrollToThumbTrackPos(bool bScrollWindow)
{
	int xPos = GetTrackPos(SB_HORZ);
	int yPos = GetTrackPos(SB_VERT);

	ScrollToPos(xPos, yPos, bScrollWindow);
}

void CTimeLineWnd::ScrollToPos(int xPos, int yPos, bool bScrollWindow)
{
	CPoint oldPos(GetScrollPos());

	SetScrollPos(SB_HORZ, xPos);
	SetScrollPos(SB_VERT, yPos);

	CPoint newPos(GetScrollPos());

	if (bScrollWindow)
	{
		// It's old-new, rather than new-old, because if we're scrolling left,
		// we need to move the window right, e.g.
		ScrollWindow(oldPos.x - newPos.x, oldPos.y - newPos.y);
	}

	// We need to recalculate/redraw the selection rectangle.
	UpdateSelectionRect();
}

int CTimeLineWnd::CalculateSpanWidthInPixels(const TimeSpan &timeSpan) const
{
	if (timeSpan.IsVisible())
	{
		FILETIME startTime = timeSpan.GetStartTime();
		FILETIME endTime = timeSpan.GetEndTime();
		
		ASSERT(startTime <= endTime);
		ULONGLONG spanDurationInSeconds = FileTimeDifferenceInSeconds(endTime, startTime);

		return ConvertSecondsToPixels(spanDurationInSeconds);
	}
	else
		return 1;
}

void CTimeLineWnd::OnPaint()
{
	CPaintDC dc(this);
	CRect clientRect;
	GetClientRect(&clientRect);

	CDC memoryDC;
	VERIFY(memoryDC.CreateCompatibleDC(&dc));

	CBitmap memoryBitmap;
	VERIFY(memoryBitmap.CreateCompatibleBitmap(&dc, clientRect.Width(), clientRect.Height()));

	CBitmap *oldBitmap = memoryDC.SelectObject(&memoryBitmap);

	DoPaint(&memoryDC);
	VERIFY(dc.BitBlt(0, 0, clientRect.Width(), clientRect.Height(), &memoryDC, 0, 0, SRCCOPY));

	memoryDC.SelectObject(oldBitmap);
}

void CTimeLineWnd::DoPaint(CDC *pDC)
{
	CFont *oldFont = pDC->SelectObject(&m_scaleFont);

	CRect clientRect;
	GetClientRect(&clientRect);
	CPoint scrollPos = GetScrollPos();

	DrawLeftMargin(pDC, clientRect, scrollPos);

	CRect remainingRect;
	DrawSpans(pDC, clientRect, scrollPos, &remainingRect);

	DrawRightMargin(pDC, remainingRect);

	pDC->SelectObject(oldFont);

	if (!m_selectionRect.IsRectEmpty())
		DrawSelection(pDC, m_selectionRect);
}

void CTimeLineWnd::DrawLeftMargin(CDC *pDC, const CRect &clientRect, const CPoint &scrollPos) const
{
	CRect leftMarginRect;
	leftMarginRect.top = clientRect.top;
	leftMarginRect.bottom = clientRect.bottom;
	leftMarginRect.left = -SCROLL_MARGIN;
	leftMarginRect.right = 0;
	leftMarginRect.OffsetRect(-scrollPos.x, 0);

	pDC->FillSolidRect(leftMarginRect, LEFT_MARGIN_COLOUR);
}

void CTimeLineWnd::DrawSpans(CDC *pDC, const CRect &clientRect, const CPoint &scrollPos, RECT *pRemaining)
{
	// spanRect is currently in client coordinates.  Adjust for the scroll position.
	CRect spanRect(clientRect);
	spanRect.OffsetRect(-scrollPos.x, 0);

	int spansDrawn = 0;

	// Recalculate the hit-test rectangles as part of the redraw:
	m_hitRects.clear();
	
	// Iterate over the spans, drawing each one.
	TimeSpanCollection::const_iterator b = m_timeSpans.begin();
	TimeSpanCollection::const_iterator e = m_timeSpans.end();
	
	// Draw the spans:
	for (TimeSpanCollection::const_iterator i = b; i != e; ++i)
	{
		spanRect.right = spanRect.left + CalculateSpanWidthInPixels(*i);

		CRect temp;
		if (temp.IntersectRect(clientRect, spanRect))
		{
			DrawSpan(pDC, spanRect, *i);
			++spansDrawn;
		}

		spanRect.left = spanRect.right;
	}

//	TRACE("%d / %d spans drawn.\n", spansDrawn, m_timeSpans.size());

	ASSERT(pRemaining);
	*pRemaining = clientRect;
	pRemaining->left = spanRect.right;
}

void CTimeLineWnd::DrawSpan(CDC *pDC, const CRect &spanRect, const TimeSpan &timeSpan)
{
	if (timeSpan.IsVisible())
		DrawVisibleSpan(pDC, spanRect, timeSpan);
	else
		DrawHiddenSpan(pDC, spanRect, timeSpan);
}

/** Given (x,y) coordinates in (time,elevation), turn them into (x,y) coordinates in pixels.
 */
CPoint CTimeLineWnd::CalculatePixelPosition(const CRect &rect, FILETIME spanStart, FILETIME thisTimestamp, double thisElevation) const
{
	int secondsSinceSpanStart = (int)FileTimeDifferenceInSeconds(thisTimestamp, spanStart);
	int x = rect.left + ConvertSecondsToPixels(secondsSinceSpanStart);

	int y = MulDiv(rect.Height(), (int)(m_maxElevation - thisElevation), (int)m_rangeElevation);
	y += rect.top;

	return CPoint(x, y);
}

/** Find the next timestamp that we'll need to draw.
 * This is worked out based on how many seconds/pixel the current scale has us drawing.
 */
FILETIME CTimeLineWnd::CalculateNextDrawableTimestamp(FILETIME thisTimestamp) const
{
	const int jumpPixels = 2;
	int jumpSeconds = (int)ConvertPixelsToSeconds(jumpPixels);
	FILETIME nextTimestamp = thisTimestamp + FileTimeFromSeconds(jumpSeconds);

	ASSERT(nextTimestamp >= thisTimestamp);
	return nextTimestamp;
}

/** Find the iterator for the next drawable position.
 */
PositionFixCollection::const_iterator
	CTimeLineWnd::FindNextDrawablePositionFix(PositionFixCollection::const_iterator i,
												PositionFixCollection::const_iterator e) const
{
	FILETIME thisTimestamp = i->GetTimestamp();
	FILETIME nextTimestamp = CalculateNextDrawableTimestamp(thisTimestamp);

	ASSERT(nextTimestamp > thisTimestamp);

	while (i != e && i->GetTimestamp() < nextTimestamp)
		++i;

	return i;
}

void CTimeLineWnd::DrawVisibleSpan(CDC *pDC, const CRect &spanRect, const TimeSpan &timeSpan)
{
	CRect scaleRect(spanRect);
	CRect bodyRect(spanRect);
	CRect footerRect(spanRect);

	scaleRect.bottom = bodyRect.top = spanRect.top + GetScaleHeight();
	bodyRect.bottom = footerRect.top = spanRect.bottom - GetFooterHeight();

	DrawVisibleSpanBody(pDC, bodyRect, timeSpan);
	DrawVisibleSpanScale(pDC, scaleRect, timeSpan);
	DrawVisibleSpanFooter(pDC, footerRect, timeSpan);
}

FILETIME RoundUpFileTimeInSeconds(FILETIME ft, ULONGLONG seconds)
{
	ULARGE_INTEGER uli = FileTimeToULargeInteger(ft);
	ULONGLONG w = seconds * FILETIME_TICKS_PER_SECOND;
	uli.QuadPart = uli.QuadPart + w - (uli.QuadPart % w);
	return ULargeIntegerToFileTime(uli);
}

int CTimeLineWnd::CalculateTickPosition(const CRect &rect, FILETIME spanStart, FILETIME thisTimestamp) const
{
	int secondsSinceSpanStart = (int)FileTimeDifferenceInSeconds(thisTimestamp, spanStart);
	int x = rect.left + ConvertSecondsToPixels(secondsSinceSpanStart);
	return x;
}

void CTimeLineWnd::DrawVisibleSpanScale(CDC *pDC, const CRect &scaleRect, const TimeSpan &timeSpan) const
{
	pDC->FillSolidRect(scaleRect, VISIBLE_SCALE_COLOUR);

	// Draw tickmarks at intervals.  Start at the beginning of the span, but round up to the interval.
	int tickIntervalSeconds = GetTickIntervalInSeconds();
//	TRACE("tickIntervalSeconds: %d\n", tickIntervalSeconds);

	FILETIME spanStart = timeSpan.GetStartTime();
	FILETIME startTime = RoundUpFileTimeInSeconds(spanStart, tickIntervalSeconds);
//	TRACE("startTime: %s\n",
//			LPCTSTR(FormatUtcFileTimeInTzSpecificLocalTime(startTime, &m_tzi)));
	
	FILETIME endTime = RoundUpFileTimeInSeconds(timeSpan.GetEndTime(), tickIntervalSeconds);
//	TRACE("endTime: %s\n",
//			LPCTSTR(FormatUtcFileTimeInTzSpecificLocalTime(endTime, &m_tzi)));

	ASSERT(startTime <= endTime);

	FILETIME tickIntervalFileTime = FileTimeFromSeconds(tickIntervalSeconds);
	for (FILETIME tickTime = startTime; tickTime < endTime; tickTime += tickIntervalFileTime)
	{
		int x = CalculateTickPosition(scaleRect, spanStart, tickTime);

//		TRACE("Drawing tick for %s at x=%d\n",
//				LPCTSTR(FormatUtcFileTimeInTzSpecificLocalTime(tickTime, &m_tzi)), x);

		CString timeStampText = GetTimestampText(tickTime);
		CSize textSize = pDC->GetTextExtent(timeStampText);

		// We need a rectangle of the correct size, centered horizontally at 'x'.
		CRect textRect(CPoint(x, 0), textSize);
		textRect.OffsetRect(-textSize.cx / 2, 0);
		textRect.bottom = scaleRect.bottom;

		// Move the actual text down a little.
		pDC->ExtTextOut(textRect.left, textRect.top + TICKMARK_HEIGHT, ETO_CLIPPED, textRect, timeStampText, NULL);

		pDC->MoveTo(x, 0);
		pDC->LineTo(x, TICKMARK_HEIGHT);
	}
}

CString CTimeLineWnd::GetTimestampText(FILETIME tickTime) const
{
	SYSTEMTIME utc;
	VERIFY(FileTimeToSystemTime(&tickTime, &utc));

	// Draw some text there.
	TIME_ZONE_INFORMATION temp(m_tzi);
	SYSTEMTIME local;
	SystemTimeToTzSpecificLocalTime(&temp, &utc, &local);

	CString timeStampText;
	GetTimeFormat(LOCALE_USER_DEFAULT, 0, &local, NULL, timeStampText.GetBuffer(_MAX_PATH), _MAX_PATH);
	timeStampText.ReleaseBuffer();

	return timeStampText;
}

int CTimeLineWnd::GetTimestampDrawThreshold() const
{
	// An arbitrary number based on the scale.
	ULONGLONG seconds = ConvertPixelsToSeconds(10);
	ASSERT(seconds <= INT_MAX);
	return (int)seconds;
}

void CTimeLineWnd::DrawVisibleSpanBody(CDC *pDC, const CRect &bodyRect, const TimeSpan &timeSpan) const
{
	pDC->FillSolidRect(bodyRect, VISIBLE_SPAN_COLOUR);

	// Find the range of position fixes covered by this span:
	FILETIME spanStart = timeSpan.GetStartTime();
	FILETIME spanEnd = timeSpan.GetEndTime();

	// TODO: Optimise for the case where we don't actually need to draw the whole
	// of 'bodyRect' -- it might not be entirely within the client area.
	// Alternatively, don't bother: just prevent spans from getting too wide.
	// This'll prevent most of the slow-down.  Just limit a span's maximum length.
	// If it's too long, chop it into smaller pieces.

	PositionFixCollection::const_iterator b = std::lower_bound(m_positionFixes->begin(), m_positionFixes->end(), spanStart, PositionsLessByTime());
	PositionFixCollection::const_iterator e = std::upper_bound(b, m_positionFixes->end(), spanEnd, PositionsLessByTime());
	ASSERT(b <= e);

	FILETIME lastTimestamp = b->GetTimestamp();
	CPoint pt = CalculatePixelPosition(bodyRect, spanStart, b->GetTimestamp(), b->GetGpsElevation());
	pDC->MoveTo(pt);

	// Now, iterate over those fixes, drawing each one:
	for (PositionFixCollection::const_iterator i = b+1; i != e;)
	{
		FILETIME thisTimestamp = i->GetTimestamp();

		CPoint pt = CalculatePixelPosition(bodyRect, spanStart, thisTimestamp, i->GetGpsElevation());

		int threshold = GetTimestampDrawThreshold();
		int deltaT = (int)FileTimeDifferenceInSeconds(thisTimestamp, lastTimestamp);

		if (deltaT > threshold)
		{
			pDC->MoveTo(pt);
			pDC->SetPixel(pt, RGB(0,0,0));
		}
		else
			pDC->LineTo(pt);

		lastTimestamp = thisTimestamp;

		// To avoid over-plotting the same pixel, we skip points until we get
		// to the next x-coordinate:
		i = FindNextDrawablePositionFix(i, e);
	}
}

/** Given (x0,x1) coordinates in time, turn them into (x0,x1) coordinates in pixels.
 */
CRect CTimeLineWnd::CalculateEventRect(const CRect &rect, FILETIME spanStart, FILETIME startTime, FILETIME endTime) const
{
	int secondsSinceSpanStart = (int)FileTimeDifferenceInSeconds(startTime, spanStart);
	int secondsInEvent = (int)FileTimeDifferenceInSeconds(endTime, startTime);
	int x0 = rect.left + ConvertSecondsToPixels(secondsSinceSpanStart);
	int x1 = x0 + ConvertSecondsToPixels(secondsInEvent);

	return CRect(x0, rect.top, x1, rect.bottom);
}

void CTimeLineWnd::DrawVisibleSpanFooter(CDC *pDC, const CRect &footerRect, const TimeSpan &timeSpan)
{
	pDC->FillSolidRect(footerRect, FOOTER_COLOUR);

	// Find those events that belong in this span.
	FILETIME spanStart = timeSpan.GetStartTime();
	FILETIME spanEnd = timeSpan.GetEndTime();

	RouteEventCollection::const_iterator begin = m_route.begin();
	RouteEventCollection::const_iterator end = m_route.end();
	RouteEventCollection::const_iterator b = std::lower_bound(begin, end, spanStart, RouteEventLessByStartTime());
	RouteEventCollection::const_iterator e = std::upper_bound(b, end, spanEnd, RouteEventLessByEndTime());
	ASSERT(b <= e);

	for (RouteEventCollection::const_iterator i = b; i != e; ++i)
	{
		CRect rect = CalculateEventRect(footerRect, spanStart, i->startTime, i->endTime);
		if (rect.Width() >= 18)	// i.e. at least room for the icon.
		{
			m_hitRects.push_back(rect);

			pDC->Rectangle(rect);
			rect.DeflateRect(1,1,1,1);

			CPoint pt(rect.TopLeft());
			m_imageList.Draw(pDC, i->imageIndex, pt, ILD_NORMAL);

			rect.DeflateRect(17, 0, 0, 0);

			CSize textSize = pDC->GetTextExtent(i->description);

			if (textSize.cx < rect.Width())
			{
				CRect textRect = CenterRectangle(rect, textSize);
				pDC->ExtTextOut(textRect.left, rect.top, ETO_CLIPPED, rect, i->description, NULL);
			}
			else
				pDC->ExtTextOut(rect.left, rect.top, ETO_CLIPPED, rect, i->description, NULL);
		}
	}
}

void CTimeLineWnd::DrawHiddenSpan(CDC *pDC, const CRect &spanRect, const TimeSpan &timeSpan) const
{
	pDC->FillSolidRect(spanRect, HIDDEN_SPAN_COLOUR);
}

void CTimeLineWnd::DrawRightMargin(CDC *pDC, const CRect &rect) const
{
	if (!rect.IsRectEmpty())
	{
		pDC->FillSolidRect(rect, RIGHT_MARGIN_COLOUR);
	}
}

BOOL CTimeLineWnd::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}

void CTimeLineWnd::OnLButtonDown(UINT nFlags, CPoint point)
{
	CWnd::OnLButtonDown(nFlags, point);

	CRect spanRect;
	FILETIME thisTime;

	if (HitTestSpans(point, &spanRect, &thisTime))
	{
		SetCapture();

		m_selectionAnchor = thisTime;
		m_selectionPoint = thisTime;
	}
}

void CTimeLineWnd::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (GetCapture() == this)
	{
		ReleaseCapture();

		CChildFrame *pFrame = static_cast<CChildFrame *>(GetParentFrame());

		// Get hold of the cartesian view.
		CCartesianView *pView = pFrame->GetCartesianView();
		if (pView)
		{
			if (m_selectionAnchor > m_selectionPoint)
				std::swap(m_selectionAnchor, m_selectionPoint);

			SelectionTimeSpan span(m_selectionAnchor, m_selectionPoint);
			pView->SelectTimeSpan(span);
		}
	}

	CWnd::OnLButtonUp(nFlags, point);
}

CRect CTimeLineWnd::GetTimeSpanRect(FILETIME startTime, FILETIME endTime)
{
	if (startTime > endTime)
		std::swap(startTime, endTime);

	// Find the left and right edges of this timespan.
	CRect rect;
	GetClientRect(&rect);
	CRect spanRect(rect);

	CPoint scrollPos = GetScrollPos();

	rect.OffsetRect(-scrollPos.x, 0);

	TimeSpanCollection::const_iterator b = m_timeSpans.begin();
	TimeSpanCollection::const_iterator e = m_timeSpans.end();
	for (TimeSpanCollection::const_iterator i = b; i != e; ++i)
	{
		FILETIME spanStart = i->GetStartTime();
		FILETIME spanEnd = i->GetEndTime();

		rect.right = rect.left + CalculateSpanWidthInPixels(*i);

		// This span covers this rectangle.  Does the span contain 'startTime'?
		if (startTime >= spanStart && startTime <= spanEnd)
		{
			// Then this span contains the left edge of the relevant rectangle.
			// Work out how far into the span the start time is:
			if (i->IsVisible())
			{
				ULONGLONG startOffsetSeconds = FileTimeDifferenceInSeconds(startTime, spanStart);
				int startOffsetPixels = ConvertSecondsToPixels(startOffsetSeconds);

				spanRect.left = rect.left + startOffsetPixels;
			}
			else
				spanRect.left = rect.left;
		}
		
		if (endTime >= spanStart && endTime <= spanEnd)
		{
			// Then this span contains the right edge of the relevant rectangle.
			if (i->IsVisible())
			{
				ULONGLONG endOffsetSeconds = FileTimeDifferenceInSeconds(endTime, spanStart);
				int endOffsetPixels = ConvertSecondsToPixels(endOffsetSeconds);

				spanRect.right = rect.left + endOffsetPixels;
			}
			else
				spanRect.right = rect.right;

			// and we can stop looking.
			break;
		}

		rect.left = rect.right;
	}

	return spanRect;
}

void CTimeLineWnd::UpdateSelectionRect()
{
	// Remove the old one.
	if (!m_selectionRect.IsRectEmpty())
		InvalidateRect(m_selectionRect, FALSE);

	if (CompareFileTime(&m_selectionAnchor, &m_selectionPoint) != 0)
	{
		CRect clientRect;
		GetClientRect(&clientRect);

		m_selectionRect = GetTimeSpanRect(m_selectionAnchor, m_selectionPoint);

		// We crop the rectangle to make it a little faster.
		// We add just a little extra: otherwise the border is drawn in the wrong place when scrolling.
		if (m_selectionRect.left < clientRect.left)
			m_selectionRect.left = clientRect.left - 1;

		if (m_selectionRect.right > clientRect.right)
			m_selectionRect.right = clientRect.right + 1;

		InvalidateRect(m_selectionRect, FALSE);
	}
}

void CTimeLineWnd::OnMouseMove(UINT nFlags, CPoint point)
{
	// If we've not already registered for WM_MOUSELEAVE messages,
	// do that now.
	if (!m_bTrackLeave)
	{
		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof(TRACKMOUSEEVENT);
		tme.hwndTrack = GetSafeHwnd();
		tme.dwFlags = TME_LEAVE;
		_TrackMouseEvent(&tme);
		m_bTrackLeave = TRUE;
	}

	CRect clientRect;
	GetClientRect(&clientRect);

	if (GetCapture() == this)
	{
		if (point.x < 10)
		{
			ScrollBy(LINE_SIZE, 0, true);
			UpdateWindow();
		}
		else if (point.x > clientRect.right - 10)
		{
			ScrollBy(LINE_SIZE, 0, true);
			UpdateWindow();
		}

		// The mouse has moved: extend/shrink the selection.
		if (point.x != m_lastMousePosition.x || point.y != m_lastMousePosition.y)
		{
			CRect spanRect;
			FILETIME thisTime;

			if (HitTestSpans(point, &spanRect, &thisTime))
			{
				// The mouse is definitely in a span, and we know the timestamp.
//				TRACE("Selection spans from %s to %s\n",
//					LPCTSTR(FormatUtcFileTimeInTzSpecificLocalTime(m_selectionAnchor, &m_tzi)),
//					LPCTSTR(FormatUtcFileTimeInTzSpecificLocalTime(thisTime, &m_tzi)));

				UpdateSelectionRect();

				m_selectionPoint = thisTime;

#define ENABLE_IMMEDIATE_SELECTION 1
#if ENABLE_IMMEDIATE_SELECTION
				CChildFrame *pFrame = static_cast<CChildFrame *>(GetParentFrame());

				// Get hold of the cartesian view.
				CCartesianView *pView = pFrame->GetCartesianView();
				if (pView)
				{
					if (m_selectionAnchor > m_selectionPoint)
						std::swap(m_selectionAnchor, m_selectionPoint);

					SelectionTimeSpan span(m_selectionAnchor, m_selectionPoint);
					pView->SelectTimeSpan(span);
				}
#endif // ENABLE_IMMEDIATE_SELECTION
			}

			UpdateWindow();
		}
	}

	CRect footerRect(clientRect);
	footerRect.top = footerRect.bottom - GetFooterHeight();
	if (footerRect.PtInRect(point))
	{
		// The mouse is in the footer.
		DeleteVerticalHighlight();
		m_timeTip.Hide();

		int hitTest = -1;
		HitRectCollection::const_iterator b = m_hitRects.begin();
		for (HitRectCollection::const_iterator i = b; i != m_hitRects.end(); ++i)
		{
			CRect rect(*i);
			if (rect.PtInRect(point))
			{
				hitTest = (int)std::distance(b, i);
				break;
			}
		}

		if (hitTest != m_lastHitTest)
		{
#if 0
			// For testing purposes, we'll highlight the hit-tested rect:
			// Remove the highlight from the previous one, and highlight the new one.
			CRect thisRect = m_hitRects[hitTest];
			CRect prevRect = m_hitRects[m_lastHitTest];

			CDC *pDC = GetDC();

			if (m_lastHitTest != -1)
				pDC->InvertRect(prevRect);
			if (hitTest != -1)
				pDC->InvertRect(thisRect);

			ReleaseDC(pDC);
#endif

			m_lastHitTest = hitTest;
		}
	}
	else
	{
		// The mouse isn't in the footer.
		if (point.x != m_lastMousePosition.x)
		{
			UpdateTrackingToolTip(point);
			DrawVerticalHighlight(point);
		}
	}

	CWnd::OnMouseMove(nFlags, point);
}

void CTimeLineWnd::DrawVerticalHighlight(CPoint point)
{
	return;

	// We higlight the time with a vertical bar, using InvertRect.
	CRect clientRect;
	GetClientRect(&clientRect);
	CDC *pDC = GetDC();

	// First, remove the old one.
	CRect oldInvert(m_lastMousePosition.x-1, 0, m_lastMousePosition.x+1, clientRect.bottom);
	pDC->InvertRect(oldInvert);
	
	// Then draw the new one.
	CRect newInvert(point.x-1, 0, point.x+1, clientRect.bottom);
	pDC->InvertRect(newInvert);

	ReleaseDC(pDC);
	m_lastMousePosition = point;
}

void CTimeLineWnd::DeleteVerticalHighlight()
{
	return;

	CRect clientRect;
	GetClientRect(&clientRect);
	CDC *pDC = GetDC();

	CRect oldInvert(m_lastMousePosition.x-1, 0, m_lastMousePosition.x+1, clientRect.bottom);
	pDC->InvertRect(oldInvert);

	ReleaseDC(pDC);
	m_lastMousePosition = CPoint(-1, -1);
}

BOOL CTimeLineWnd::HitTestSpans(CPoint point, CRect *pSpanRect, FILETIME *pFileTime)
{
	// Find out which span the mouse is over.
	CRect rect;
	GetClientRect(&rect);

	CPoint scrollPos = GetScrollPos();

	rect.OffsetRect(-scrollPos.x, 0);

	TimeSpanCollection::const_iterator b = m_timeSpans.begin();
	TimeSpanCollection::const_iterator e = m_timeSpans.end();
	for (TimeSpanCollection::const_iterator i = b; i != e; ++i)
	{
		rect.right = rect.left + CalculateSpanWidthInPixels(*i);

		if (rect.PtInRect(point))
		{
			// Found it.

			// How far across the span in pixels are we?
			int xOffset = point.x - rect.left;

			// And in seconds?
			ULONGLONG seconds = ConvertPixelsToSeconds(xOffset);

			// What's that as an actual timestamp?
			FILETIME startTime = i->GetStartTime();

			FILETIME increment = FileTimeFromSeconds((unsigned)seconds);
			FILETIME thisTime = startTime + increment;

			ULONGLONG temp = FileTimeDifferenceInSeconds(thisTime, startTime);
			ASSERT(temp == seconds);
			
			*pSpanRect = rect;
			*pFileTime = thisTime;
			return TRUE;
		}

		rect.left = rect.right;
	}

	return FALSE;
}

void CTimeLineWnd::UpdateTrackingToolTip(CPoint point)
{
	CRect spanRect;
	FILETIME thisTime;
	if (HitTestSpans(point, &spanRect, &thisTime))
	{
#if 0
		CString s;
		s.Format("Span #%d\n(%s - %s)\n%d (%I64u)\n%s",
			std::distance(b, i),
			LPCTSTR(FormatUtcFileTimeInTzSpecificLocalTime(i->GetStartTime(), &m_tzi)),
			LPCTSTR(FormatUtcFileTimeInTzSpecificLocalTime(i->GetEndTime(), &m_tzi)),
			xOffset, seconds,
			LPCTSTR(FormatUtcFileTimeInTzSpecificLocalTime(thisTime, &m_tzi)));
#else
		CString s = FormatUtcFileTimeInTzSpecificLocalTime(thisTime, &m_tzi);
#endif

		CPoint pt(point);
		ClientToScreen(&pt);
		pt.Offset(25, 25);

		m_timeTip.Show(this, pt, s);
	}
}

/** A WM_MOUSELEAVE message is sent when the mouse leaves the window.
 * It's also sent when a menu is opened with the keyboard.  The mouse
 * may not have left the window, so we need to deal with this.
 */
LRESULT CTimeLineWnd::OnMouseLeave(WPARAM wParam, LPARAM lParam)
{
    m_bTrackLeave = FALSE;

	DeleteVerticalHighlight();
	m_timeTip.Hide();
    return 0;
}


/** We're sent this when the menu discussed in OnMouseLeave is closed.
 * Since we won't have received any further WM_MOUSELEAVE messages,
 * we need to check whether the mouse has left the window.
 *
 * Unfortunately, this message is only sent to top-level windows,
 * so we need our main window to tell us about it, using SendMessageToDescendants.
 */
void CTimeLineWnd::OnExitMenuLoop(BOOL bIsTrackPopupMenu)
{
	DeleteVerticalHighlight();
	m_timeTip.Hide();
}

BOOL CTimeLineWnd::PreTranslateMessage(MSG* pMsg)
{
	m_timeTip.PreTranslateMessage(pMsg);

	return CWnd::PreTranslateMessage(pMsg);
}

void CTimeLineWnd::HideSpans()
{
	RecalcSpans();
	UpdateScrollBar(TRUE);
	ScrollToPos(0, 0, false);
	Invalidate();
}

void CTimeLineWnd::ShowAllSpans()
{
	for (TimeSpanCollection::iterator i = m_timeSpans.begin(); i != m_timeSpans.end(); ++i)
	{
		i->SetVisible(true);
	}

	UpdateScrollBar(TRUE);
	Invalidate();
}
