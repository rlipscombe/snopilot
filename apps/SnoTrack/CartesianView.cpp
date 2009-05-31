// CartesianView.cpp : implementation file
//

#include "stdafx.h"
#include "CartesianView.h"
#include "resource.h"
#include "SnoTrackDoc.h"
#include "ChildFrm.h"
#include "win32_ui/bold_font.h"
#include <algorithm>
#include "ViewOptionsDialog.h"
#include "win32_ui/center_rectangle.h"
#include "GridLinesLayer.h"
#include "LogicalRangeLayer.h"
#include "DeviceRangeLayer.h"
#include "TrackLogLayer.h"
#include "PathLayer.h"
#include "LiftsLayer.h"
#include "WaypointsLayer.h"
#include "HotspotGridLayer.h"
#include "ScaleLayer.h"
#include "LatLon.h"
#include "DrawSite.h"
#include "GridBounds.h"
#include "IntroView.h"
#include <numeric>
#include "win32/filetime_helpers.h"
#include "CalcRoute.h"
#include "HotspotSizeDialog.h"

// CCartesianView
CString FormatMemoryCapacity(INT64 bytes)
{
	TCHAR buffer[MAX_PATH];
	StrFormatByteSize64(bytes, buffer, COUNTOF(buffer));
	return buffer;
}

IMPLEMENT_DYNCREATE(CCartesianView, CView)

CCartesianView::CCartesianView()
	: m_scaleFactor(32), m_clickAction(ON_CLICK_CENTER)
#if defined(ENABLE_DESIGN_MODE)
	, m_hotspotLayer(NULL), m_pathLayer(NULL), m_hotspotSize(1)
#endif
{
}

CCartesianView::~CCartesianView()
{
}

BEGIN_MESSAGE_MAP(CCartesianView, CView)
	ON_WM_SIZE()
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_WM_MOUSEWHEEL()
	ON_WM_ERASEBKGND()
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONUP()
	ON_WM_KEYUP()
	ON_WM_KEYDOWN()
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_LON, OnUpdateLongitude)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_LAT, OnUpdateLatitude)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_SCALE, OnUpdateScale)
	ON_COMMAND(ID_VIEW_ZOOMIN, OnViewZoomin)
	ON_UPDATE_COMMAND_UI(ID_VIEW_ZOOMIN, OnUpdateViewZoomin)
	ON_COMMAND(ID_VIEW_ZOOMOUT, OnViewZoomout)
	ON_UPDATE_COMMAND_UI(ID_VIEW_ZOOMOUT, OnUpdateViewZoomout)
	ON_COMMAND(ID_VIEW_ROTATECCW, OnViewRotateccw)
	ON_COMMAND(ID_VIEW_ROTATECW, OnViewRotatecw)
	ON_COMMAND(ID_MODE_ZOOMIN, OnModeZoomin)
	ON_UPDATE_COMMAND_UI(ID_MODE_ZOOMIN, OnUpdateModeZoomin)
	ON_COMMAND(ID_MODE_ZOOMOUT, OnModeZoomout)
	ON_UPDATE_COMMAND_UI(ID_MODE_ZOOMOUT, OnUpdateModeZoomout)
	ON_COMMAND(ID_MODE_CENTER, OnModeCenter)
	ON_UPDATE_COMMAND_UI(ID_MODE_CENTER, OnUpdateModeCenter)
	ON_WM_MOUSEMOVE()
	ON_WM_PAINT()
	ON_COMMAND(ID_VIEW_OPTIONS, OnViewOptions)
	ON_WM_CREATE()
	ON_WM_DESTROY()

#if defined(ENABLE_DESIGN_MODE)
	ON_COMMAND(ID_HOTSPOTS_DEFINEHOTSPOTS, OnHotspotsDefinehotspots)
	ON_UPDATE_COMMAND_UI(ID_HOTSPOTS_DEFINEHOTSPOTS, OnUpdateHotspotsDefinehotspots)
	ON_COMMAND(ID_HOTSPOTS_CLEARHOTSPOTS, OnHotspotsClearhotspots)
	ON_COMMAND(ID_RESORT_HIDELIFTS, OnResortHidelifts)
	ON_COMMAND(ID_RESORT_HIDEMATCHEDPATHS, OnResortHidematchedpaths)
	ON_COMMAND(ID_RESORT_SHOWALL, OnResortShowall)
	ON_COMMAND(ID_RESORT_HOTSPOTSIZE, OnResortHotspotsize)
#endif
END_MESSAGE_MAP()


// CCartesianView drawing
#define USE_OFFSCREEN_DC 1

void CCartesianView::OnDraw(CDC *pDC, const PAINTSTRUCT *ps)
{
#if USE_OFFSCREEN_DC
	CRect clientRect;
	GetClientRect(&clientRect);

	CDC memoryDC;
	memoryDC.CreateCompatibleDC(pDC);
	
	CBitmap memoryBitmap;
	memoryBitmap.CreateCompatibleBitmap(pDC, clientRect.Width(), clientRect.Height());

	CBitmap *oldBitmap = memoryDC.SelectObject(&memoryBitmap);
	
	memoryDC.FillSolidRect(clientRect, RGB(255,255,255));

	DoDraw(&memoryDC, ps);

	pDC->BitBlt(0, 0, clientRect.Width(), clientRect.Height(), &memoryDC, 0, 0, SRCCOPY);
	memoryDC.SelectObject(oldBitmap);
#else
	CRect paintRect(ps->rcPaint);
	pDC->PatBlt(paintRect.left, paintRect.top, paintRect.Width(), paintRect.Height(), WHITENESS);
	DoDraw(pDC, ps);
#endif
}

void AssertLogicalPointInRange(CPoint logical)
{
	// Due to the way that the rotating works, logical points must be no more than
	// INT_MAX / sqrt(2).  Due to the way that the scrolling works, this is halved to
	// INT_MAX / 2sqrt(2).
	// As an integer, this comes out at 759,250,124.
	const int MAGIC_NUMBER = 759250124;

	ASSERT(-MAGIC_NUMBER <= logical.x && logical.x <= MAGIC_NUMBER);
	ASSERT(-MAGIC_NUMBER <= logical.y && logical.y <= MAGIC_NUMBER);
}

CPoint CCartesianView::GetScrollPos()
{
	// Avoid calling into the MFC DLL for this: The profiler reckons it'll make it quicker.
	return CPoint(::GetScrollPos(m_hWnd, SB_HORZ), ::GetScrollPos(m_hWnd, SB_VERT));
}

/** Transform from logical points to window points, by applying rotation.
 */
CPoint CCartesianView::GetWindowPointFromLogicalPoint(CPoint logical)
{
	// Apply the rotation here:
	return m_rotation.Forward(logical);
}

CPoint CCartesianView::GetLogicalPointFromWindowPoint(CPoint window)
{
	// Apply the negative rotation here:
	return m_rotation.Reverse(window);
}

/** Transform from window points to device points, by inverting the y-axis.
 */
CPoint CCartesianView::GetDevicePointFromWindowPoint(CPoint window)
{
	return CPoint(window.x / m_scaleFactor, -window.y / m_scaleFactor);
}

/** Transform from device points to client points, by scrolling as relevant.
 */
CPoint CCartesianView::GetClientPointFromDevicePoint(CPoint device)
{
	CPoint scrollPos(GetScrollPos());
	return device - scrollPos;
}

CPoint CCartesianView::GetClientPointFromLogicalPoint(CPoint logical)
{
	AssertLogicalPointInRange(logical);

	CPoint window(GetWindowPointFromLogicalPoint(logical));
	CPoint device(GetDevicePointFromWindowPoint(window));
	CPoint client(GetClientPointFromDevicePoint(device));

	return client;
}

CPoint CCartesianView::GetDevicePointFromLogicalPoint(CPoint logical)
{
	AssertLogicalPointInRange(logical);

	CPoint window(GetWindowPointFromLogicalPoint(logical));
	CPoint device(GetDevicePointFromWindowPoint(window));

	return device;
}

CPoint CCartesianView::GetWindowPointFromDevicePoint(CPoint device)
{
	return CPoint(device.x * m_scaleFactor, -device.y * m_scaleFactor);
}

CPoint CCartesianView::GetDevicePointFromClientPoint(CPoint client)
{
	CPoint scrollPos(GetScrollPos());
	return client + scrollPos;
}

CPoint CCartesianView::GetWindowPointFromClientPoint(CPoint client)
{
	CPoint device(GetDevicePointFromClientPoint(client));
	CPoint window(GetWindowPointFromDevicePoint(device));

	return window;
}

CPoint CCartesianView::GetLogicalPointFromClientPoint(CPoint client)
{
	CPoint device(GetDevicePointFromClientPoint(client));
	CPoint window(GetWindowPointFromDevicePoint(device));
	CPoint logical(GetLogicalPointFromWindowPoint(window));

	return logical;
}

class CartesianViewDrawSite : public DrawSite
{
	CCartesianView *m_pView;
	CRect m_invalidRect;
	CRect m_clientRect;
	CSnoTrackDoc *m_pDoc;
	const Projection *m_proj;
	int m_scaleFactor;
	Rotation m_rotation;
	CPoint m_scrollPos;

public:
	CartesianViewDrawSite(CCartesianView *pView, CRect invalidRect, CRect clientRect,
							CSnoTrackDoc* pDoc, const Projection *proj, int scaleFactor,
							Rotation rotation, CPoint scrollPos)
		: m_pView(pView), m_invalidRect(invalidRect), m_clientRect(clientRect),
			m_pDoc(pDoc), m_proj(proj), m_scaleFactor(scaleFactor),
			m_rotation(rotation), m_scrollPos(scrollPos)
	{
	}

// DrawSite
public:
	virtual CRect GetInvalidRect() const
	{
		return m_invalidRect;
	}

	virtual CRect GetClientRect() const
	{
		return m_clientRect;
	}

	virtual GridBounds GetBounds()
	{
		return m_pDoc->GetBounds();
	}

	virtual int GetScaleFactor()
	{
		return m_scaleFactor;
	}

	virtual Rotation GetRotation()
	{
		return m_rotation;
	}

	virtual CPoint GetClientPointFromSphere(double lon, double lat)
	{
		CPoint logical(m_proj->ForwardDeg(lon, lat));
		CPoint client(m_pView->GetClientPointFromLogicalPoint(logical));

		return client;
	}

	virtual CRect GetLogicalRange() { return m_pView->GetLogicalRange(); }

	virtual CPoint GetClientPointFromLogicalPoint(CPoint logical)
	{
		CPoint device(m_pView->GetDevicePointFromLogicalPoint(logical));
		CPoint client(GetClientPointFromDevicePoint(device));

		return client;
	}

	virtual CPoint GetLogicalPointFromClientPoint(CPoint client) { return m_pView->GetLogicalPointFromClientPoint(client); }
	virtual CRect GetDeviceRange() { return m_pView->GetDeviceRange(); }

	virtual CPoint GetClientPointFromDevicePoint(CPoint device)
	{
		return device - m_scrollPos;
	}
};

void CCartesianView::DoDraw(CDC *pDC, const PAINTSTRUCT *ps)
{
	DWORD startTime = GetTickCount();

	CSnoTrackDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	if (!pDoc->IsEmptyDataSet())
	{
		CRect clientRect;
		GetClientRect(&clientRect);

		/// @todo redrawRect, really.
		CRect invalidRect = (ps ? ps->rcPaint : clientRect);

		CartesianViewDrawSite drawSite(this, invalidRect, clientRect, pDoc, pDoc->GetProjection(), m_scaleFactor, m_rotation, GetScrollPos());

		for (Layers::const_iterator i = m_layers.begin(); i != m_layers.end(); ++i)
		{
			Layer *layer = *i;

			if (layer->IsEnabled())
				layer->OnDraw(&drawSite, pDC);
		}
	}

	DWORD endTime = GetTickCount();
	TRACE("DoDraw took %d ms\n", endTime - startTime);
}
// CCartesianView diagnostics

#ifdef _DEBUG
void CCartesianView::AssertValid() const
{
	CView::AssertValid();
}

void CCartesianView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CSnoTrackDoc* CCartesianView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CSnoTrackDoc)));
	return (CSnoTrackDoc*)m_pDocument;
}
#endif //_DEBUG

BOOL CCartesianView::PreCreateWindow(CREATESTRUCT& cs)
{
	if (!CView::PreCreateWindow(cs))
		return FALSE;

	cs.style |= WS_VSCROLL | WS_HSCROLL;
	cs.dwExStyle |= WS_EX_CLIENTEDGE;

	return TRUE;
}

void CCartesianView::RecalcAllRanges()
{
	CWaitCursor wait;
	DWORD startTime = GetTickCount();

	CSnoTrackDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	if (pDoc->IsEmptyDataSet())
	{
		m_logicalRange = CRect(0, 0, 0, 0);
		m_windowRange = CRect(0, 0, 0, 0);
	}
	else
	{
		m_logicalRange = CRect(INT_MAX, INT_MAX, -INT_MAX, -INT_MAX);
		m_windowRange = CRect(INT_MAX, INT_MAX, -INT_MAX, -INT_MAX);
		m_deviceRange = CRect(INT_MAX, INT_MAX, -INT_MAX, -INT_MAX);

		for (PositionFixCollection::const_iterator i = pDoc->m_positions.begin(); i != pDoc->m_positions.end(); ++i)
		{
			CPoint logical(i->GetProjectionX(), i->GetProjectionY());

			m_logicalRange.left = min(m_logicalRange.left, logical.x);
			m_logicalRange.top = min(m_logicalRange.top, logical.y);
			m_logicalRange.right = max(m_logicalRange.right, logical.x);
			m_logicalRange.bottom = max(m_logicalRange.bottom, logical.y);

			CPoint window(GetWindowPointFromLogicalPoint(logical));

			m_windowRange.left = min(m_windowRange.left, window.x);
			m_windowRange.top = min(m_windowRange.top, window.y);
			m_windowRange.right = max(m_windowRange.right, window.x);
			m_windowRange.bottom = max(m_windowRange.bottom, window.y);
		}

		for (WaypointCollection::const_iterator i = pDoc->m_user_waypoints.begin(); i != pDoc->m_user_waypoints.end(); ++i)
		{
			CPoint logical(i->GetProjectionX(), i->GetProjectionY());

			m_logicalRange.left = min(m_logicalRange.left, logical.x);
			m_logicalRange.top = min(m_logicalRange.top, logical.y);
			m_logicalRange.right = max(m_logicalRange.right, logical.x);
			m_logicalRange.bottom = max(m_logicalRange.bottom, logical.y);

			CPoint window(GetWindowPointFromLogicalPoint(logical));

			m_windowRange.left = min(m_windowRange.left, window.x);
			m_windowRange.top = min(m_windowRange.top, window.y);
			m_windowRange.right = max(m_windowRange.right, window.x);
			m_windowRange.bottom = max(m_windowRange.bottom, window.y);
		}

		for (WaypointCollection::const_iterator i = pDoc->m_resort_waypoints.begin(); i != pDoc->m_resort_waypoints.end(); ++i)
		{
			CPoint logical(i->GetProjectionX(), i->GetProjectionY());

			m_logicalRange.left = min(m_logicalRange.left, logical.x);
			m_logicalRange.top = min(m_logicalRange.top, logical.y);
			m_logicalRange.right = max(m_logicalRange.right, logical.x);
			m_logicalRange.bottom = max(m_logicalRange.bottom, logical.y);

			CPoint window(GetWindowPointFromLogicalPoint(logical));

			m_windowRange.left = min(m_windowRange.left, window.x);
			m_windowRange.top = min(m_windowRange.top, window.y);
			m_windowRange.right = max(m_windowRange.right, window.x);
			m_windowRange.bottom = max(m_windowRange.bottom, window.y);
		}

		// RECTs are endpoint-exclusive, so bump the bottom right.
		m_logicalRange.right++;
		m_logicalRange.bottom++;

		NotifyRecalcLayers();
	}

	RecalcDeviceRange();

	DWORD endTime = GetTickCount();
	TRACE("RecalcAllRanges took %d ms.\n", endTime - startTime);
}

void CCartesianView::NotifyRecalcLayers()
{
	// We also need to update the layers when the ranges are recalculated.
	// A better way to do this might be to pass 'this' to the constructor of the layer,
	// and then they can register to observe the change.  This would mean that we
	// wouldn't have to put together a DrawSite object at this point.
	// Particularly because we've not got all of the information required for one
	// (since we're not actually drawing right now).
	CSnoTrackDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	CRect logicalRange = GetLogicalRange();
	for (Layers::const_iterator i = m_layers.begin(); i != m_layers.end(); ++i)
	{
		Layer *layer = *i;

		if (layer->IsEnabled())
			layer->OnRecalc(logicalRange);
	}
}

void CCartesianView::NotifySelectLayers()
{
	CRect logicalRange = GetLogicalRange();
	for (Layers::const_iterator i = m_layers.begin(); i != m_layers.end(); ++i)
	{
		Layer *layer = *i;

		if (layer->IsEnabled())
			layer->OnSelect(m_selection);
	}
}

void CCartesianView::RecalcWindowRange()
{
	CSnoTrackDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	if (pDoc->IsEmptyDataSet())
	{
		m_windowRange = CRect(0, 0, 0, 0);
	}
	else
	{
		m_windowRange = CRect(INT_MAX, INT_MAX, -INT_MAX, -INT_MAX);

		for (PositionFixCollection::const_iterator i = pDoc->m_positions.begin(); i != pDoc->m_positions.end(); ++i)
		{
			CPoint logical(i->GetProjectionX(), i->GetProjectionY());

			CPoint window(GetWindowPointFromLogicalPoint(logical));

			m_windowRange.left = min(m_windowRange.left, window.x);
			m_windowRange.top = min(m_windowRange.top, window.y);
			m_windowRange.right = max(m_windowRange.right, window.x);
			m_windowRange.bottom = max(m_windowRange.bottom, window.y);
		}

		for (WaypointCollection::const_iterator i = pDoc->m_user_waypoints.begin(); i != pDoc->m_user_waypoints.end(); ++i)
		{
			CPoint logical(i->GetProjectionX(), i->GetProjectionY());

			CPoint window(GetWindowPointFromLogicalPoint(logical));

			m_windowRange.left = min(m_windowRange.left, window.x);
			m_windowRange.top = min(m_windowRange.top, window.y);
			m_windowRange.right = max(m_windowRange.right, window.x);
			m_windowRange.bottom = max(m_windowRange.bottom, window.y);
		}

		for (WaypointCollection::const_iterator i = pDoc->m_resort_waypoints.begin(); i != pDoc->m_resort_waypoints.end(); ++i)
		{
			CPoint logical(i->GetProjectionX(), i->GetProjectionY());

			CPoint window(GetWindowPointFromLogicalPoint(logical));

			m_windowRange.left = min(m_windowRange.left, window.x);
			m_windowRange.top = min(m_windowRange.top, window.y);
			m_windowRange.right = max(m_windowRange.right, window.x);
			m_windowRange.bottom = max(m_windowRange.bottom, window.y);
		}
	}
}

void CCartesianView::RecalcDeviceRange()
{
	// Work out the device range from the window range.  The window range isn't affected by scaling, so can be cached:
	CPoint deviceTopLeft(GetDevicePointFromWindowPoint(CPoint(m_windowRange.left, m_windowRange.bottom)));
	CPoint deviceBottomRight(GetDevicePointFromWindowPoint(CPoint(m_windowRange.right, m_windowRange.top)));

	m_deviceRange = CRect(deviceTopLeft, deviceBottomRight);

//		m_deviceRange.right++;
//		m_deviceRange.bottom++;

//	TRACE("m_deviceRange = (%d, %d, %d, %d)\n", m_deviceRange.left, m_deviceRange.top, m_deviceRange.right, m_deviceRange.bottom);
}

void CCartesianView::UpdateScrollBars()
{
	CRect scrollRange = GetDeviceRange();

	CSnoTrackDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	if (pDoc->IsEmptyDataSet())
	{
		SetScrollRange(SB_HORZ, 0, 0, TRUE);
		SetScrollRange(SB_VERT, 0, 0, TRUE);
	}
	else
	{
		// Widen the scroll range to allow overscroll:
		CRect clientRect;
		GetClientRect(&clientRect);

		scrollRange.left -= clientRect.Width();
		scrollRange.top -= clientRect.Height();

		// Ensure that the scroll range rectangle doesn't get turned inside-out:
		if (scrollRange.right < scrollRange.left)
			scrollRange.right = scrollRange.left;
		if (scrollRange.bottom < scrollRange.top)
			scrollRange.bottom = scrollRange.top;

		SetScrollRange(SB_HORZ, scrollRange.left, scrollRange.right, TRUE);
		SetScrollRange(SB_VERT, scrollRange.top, scrollRange.bottom, TRUE);
	}
}

void CCartesianView::ScrollBy(int xScroll, int yScroll, bool bScrollWindow)
{
//	TRACE("ScrollBy(%d, %d)\n", xScroll, yScroll);
	CPoint oldPos(GetScrollPos());
	ScrollToPos(oldPos.x + xScroll, oldPos.y + yScroll, bScrollWindow);
}

int CCartesianView::GetTrackPos(int nBar)
{
	SCROLLINFO si;
	memset(&si, 0, sizeof(SCROLLINFO));
	si.cbSize = sizeof(SCROLLINFO);
	GetScrollInfo(nBar, &si, SIF_TRACKPOS);

	return si.nTrackPos;
}

void CCartesianView::ScrollToThumbTrackPos(bool bScrollWindow)
{
	int xPos = GetTrackPos(SB_HORZ);
	int yPos = GetTrackPos(SB_VERT);

	ScrollToPos(xPos, yPos, bScrollWindow);
}

void CCartesianView::ScrollToPos(int xPos, int yPos, bool bScrollWindow)
{
//	TRACE("ScrollToPos(%d, %d)\n", xPos, yPos);
	CPoint oldPos(GetScrollPos());

	SetScrollPos(SB_HORZ, xPos);
	SetScrollPos(SB_VERT, yPos);

	CPoint newPos(GetScrollPos());

	if (bScrollWindow)
	{
		// It's old-new, rather than new-old, because if we're scrolling left,
		// we need to move the window right, e.g.
		int xAmount = oldPos.x - newPos.x;
		int yAmount = oldPos.y - newPos.y;

		for (Layers::const_iterator i = m_layers.begin(); i != m_layers.end(); ++i)
		{
			Layer *layer = *i;
			layer->OnScroll(this, xAmount, yAmount);
		}

		ScrollWindow(xAmount, yAmount);
	}
}

void CCartesianView::ScrollToCenter()
{
	int minX, maxX, minY, maxY;
	GetScrollRange(SB_HORZ, &minX, &maxX);
	GetScrollRange(SB_VERT, &minY, &maxY);

	CRect scrollRange(minX, minY, maxX, maxY);

	ScrollToPos(scrollRange.CenterPoint(), true);
}

const int MAX_SCALE_FACTOR = 262144;

int CalculateIdealScale(const CRect &range, const CRect &clientRect)
{
	if (!range.IsRectEmpty() && !clientRect.IsRectEmpty())
	{
		double xRatio = range.Width() / clientRect.Width();
		double yRatio = range.Height() / clientRect.Height();

		// We need to find the closest power of two to the ideal scale.
		double idealScale = max(xRatio, yRatio);
		for (int f = 1; f < MAX_SCALE_FACTOR; f *= 2)
		{
			if (f >= idealScale)
				return f;
		}
	}

	return 32;
}

void CCartesianView::CenterOnWindowRange(const CRect &windowRange)
{
//	TRACE("CenterOnWindowRange(left = %d, top = %d, right = %d, bottom = %d)\n",
//		windowRange.left, windowRange.top, windowRange.right, windowRange.bottom);

	CRect clientRect;
	GetClientRect(&clientRect);

	m_scaleFactor = CalculateIdealScale(windowRange, clientRect);

	RecalcDeviceRange();
	UpdateScrollBars();

	CenterOnWindowPoint(windowRange.CenterPoint());
}

template <typename PrHit, typename PrMiss>
void CorrelatePositionFixes(PositionFixCollection::iterator p_begin, PositionFixCollection::iterator p_end,
							SelectionTimeSpanCollection::const_iterator s_begin, SelectionTimeSpanCollection::const_iterator s_end,
							PrHit hit, PrMiss miss)
{
	SelectionTimeSpanCollection::const_iterator s = s_begin;

	// Iterate over the position fixes, highlighting them.
	for (PositionFixCollection::iterator i = p_begin; i != p_end; ++i)
	{
		if (s == s_end)
		{
			// We've got no more selection intervals.  This point should be drawn normally.
			miss(*i);
		}
		else if (i->GetTimestamp() < s->m_startTime)
		{
			// This point comes before the current selection interval:
			// it should be drawn normally.
			miss(*i);
		}
		else if (i->GetTimestamp() < s->m_endTime)
		{
			// Then this point falls in the current selection interval:
			// it should be highlighted:
			hit(*i);
		}
		else
		{
			// Then this point comes after the current selection interval.
			// Find the next selection interval that includes this point:
			while (s != s_end && i->GetTimestamp() >= s->m_startTime)
				++s;

			// Did we find a selection that includes this point?
			if (i->GetTimestamp() >= s->m_startTime && i->GetTimestamp() < s->m_endTime)
			{
				hit(*i);
			}
			else
			{
				miss(*i);
			}
		}
	}
}

class AccumulateRange
{
	CCartesianView *m_pView;
	CRect *m_pRange;

public:
	AccumulateRange(CCartesianView *pView, CRect *pRange)
		: m_pView(pView), m_pRange(pRange)
	{
	}

	void operator() (PositionFix &positionFix)
	{
		positionFix.SetStyle(PositionFix::STYLE_HIGHLIGHT);

		// It also contributes to the selection range:
		CPoint logical(positionFix.GetProjectionX(), positionFix.GetProjectionY());
		CPoint window(m_pView->GetWindowPointFromLogicalPoint(logical));

		m_pRange->left = min(m_pRange->left, window.x);
		m_pRange->top = min(m_pRange->top, window.y);
		m_pRange->right = max(m_pRange->right, window.x);
		m_pRange->bottom = max(m_pRange->bottom, window.y);
	}
};

class ApplyStyle
{
	PositionFix::Style m_style;

public:
	ApplyStyle(PositionFix::Style style)
		: m_style(style)
	{
	}

	void operator() (PositionFix &positionFix)
	{
		positionFix.SetStyle(m_style);
	}
};

void CCartesianView::SelectTimeSpan(const SelectionTimeSpan &span)
{
	SelectionTimeSpanCollection spans;
	spans.push_back(span);

	SelectTimeSpans(spans);
}

void CCartesianView::SelectTimeSpans(const SelectionTimeSpanCollection &selection)
{
//	DWORD startTime = GetTickCount();

	m_selection = selection;
	std::sort(m_selection.begin(), m_selection.end(), SelectionTimeSpanLess());

	CRect range;
	HighlightPositionFixes(&range);

	NotifySelectLayers();

	if (selection.size())
		CenterOnWindowRange(range);

//	DWORD endTime = GetTickCount();
//	TRACE("SelectTimeSpans took %d ms.\n", endTime - startTime);
}

void CCartesianView::HighlightPositionFixes(CRect *pRange)
{
//	DWORD startTime = GetTickCount();

	// Ensure that the selections are sorted:
	CSnoTrackDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	CRect range(INT_MAX, INT_MAX, -INT_MAX, -INT_MAX);

	PositionFixCollection::iterator p_begin = pDoc->m_positions.begin();
	PositionFixCollection::iterator p_end = pDoc->m_positions.end();
	CorrelatePositionFixes(p_begin, p_end, m_selection.begin(), m_selection.end(), AccumulateRange(this, &range),
							ApplyStyle(m_selection.empty() ? PositionFix::STYLE_NORMAL : PositionFix::STYLE_FADED));

	*pRange = range;

//	DWORD endTime = GetTickCount();
//	TRACE("HighlightPositionFixes took %d ms.\n", endTime - startTime);
}

/** Restore the view to "sensible" defaults.
 * Currently, this includes:
 *  default (resort) rotation.
 *  zoom/center on the last N points in the tracklog.
 */
void CCartesianView::SetDefaultViewProperties()
{
	CSnoTrackDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// Orient the tracklog according to the resort.
	DWORD dwDefaultOrientation = pDoc->GetDefaultOrientation();

	m_rotation.theta = DegreesToRadians(dwDefaultOrientation);
	m_rotation.sin_theta = sin(m_rotation.theta);
	m_rotation.cos_theta = cos(m_rotation.theta);

	TRACE("Rotating view to %f degrees.\n", RadiansToDegrees(m_rotation.theta));

	// We've added/removed data points.  We need to recalculate the logical, window and device ranges.
	RecalcAllRanges();

	// Iterate over the *last* N position fixes.  We need to work out how to place them on the screen.
	// We need a centre-of-gravity algorithm for them, and we need to zoom accordingly.

	if (!pDoc->m_positions.empty())
	{
		const int MAGIC_NUMBER = 2500;
		PositionFixCollection::const_iterator b = pDoc->m_positions.begin();
		if (pDoc->m_positions.size() > MAGIC_NUMBER)
			b = pDoc->m_positions.end() - MAGIC_NUMBER;
		PositionFixCollection::const_iterator e = pDoc->m_positions.end();

		CRect range(INT_MAX, INT_MAX, -INT_MAX, -INT_MAX);

		for (PositionFixCollection::const_iterator i = b; i != e; ++i)
		{
			CPoint logical(i->GetProjectionX(), i->GetProjectionY());
			CPoint window(GetWindowPointFromLogicalPoint(logical));

			range.left = min(range.left, window.x);
			range.top = min(range.top, window.y);
			range.right = max(range.right, window.x);
			range.bottom = max(range.bottom, window.y);
		}

		CenterOnWindowRange(range);
	}
	else
	{
		RecalcDeviceRange();
		UpdateScrollBars();
		ScrollToCenter();
	}

	Invalidate();
}

void CCartesianView::ScrollToLogicalPoint(CPoint logicalPoint, CPoint clientPoint)
{
	CPoint oldDevicePoint(GetDevicePointFromLogicalPoint(logicalPoint));
	CPoint newDevicePoint(GetDevicePointFromClientPoint(clientPoint));

	ScrollToDevicePoint(oldDevicePoint, newDevicePoint);
}

/** Scroll the view so that 'windowPoint' is at 'clientPoint' in the client area.
 */
void CCartesianView::ScrollToWindowPoint(CPoint windowPoint, CPoint clientPoint)
{
	CPoint oldDevicePoint(GetDevicePointFromWindowPoint(windowPoint));
	CPoint newDevicePoint(GetDevicePointFromClientPoint(clientPoint));

	ScrollToDevicePoint(oldDevicePoint, newDevicePoint);
}

/** Magic.  See the comments.
 */
void CCartesianView::ScrollToDevicePoint(CPoint oldDevicePoint, CPoint newDevicePoint)
{
	// Ordinarily, when working out how to get from one position to another, we'd use
	// (new-old).  Here, we're using (old-new).  Why's that?
	// Well, it's because the names are confusing.
	// We know where the mouse _was_ under the old scaling, in window coordinates: r
	// We know where the mouse is under the new scaling, in device/window coordinates: newDevicePoint.
	// We know where the old position is under the new scaling, in device coordinates: oldDevicePoint.
	// The naming is kinda backwards: we're actually trying to get from newDevicePoint (where the mouse is now)
	// to oldDevicePoint (where the mouse should be).  Hence why it's backwards.
	ScrollBy(oldDevicePoint.x - newDevicePoint.x, oldDevicePoint.y - newDevicePoint.y, false);

	Invalidate();
}

void CCartesianView::CenterOnWindowPoint(CPoint pt)
{
	CRect clientRect;
	GetClientRect(&clientRect);

	ScrollToWindowPoint(pt, clientRect.CenterPoint());
}

void CCartesianView::OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/)
{
	CSnoTrackDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	if (pDoc->IsEmptyDataSet())
	{
		CChildFrame *pFrame = static_cast<CChildFrame *>(GetParentFrame());
		pFrame->DeferReplaceView(RUNTIME_CLASS(CIntroView));
	}
	else
	{
		SetDefaultViewProperties();
	}
}

void CCartesianView::OnSize(UINT nType, int cx, int cy)
{
	if (nType != SIZE_MINIMIZED)
	{
		UpdateScrollBars();
	}

	CView::OnSize(nType, cx, cy);
}

const int LINE_SIZE = 10;
const int PAGE_SIZE = 100;

void CCartesianView::OnHScroll(UINT nSBCode, UINT /*nPos*/, CScrollBar* pScrollBar)
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

void CCartesianView::OnVScroll(UINT nSBCode, UINT /*nPos*/, CScrollBar* pScrollBar)
{
	if (pScrollBar != NULL && pScrollBar->SendChildNotifyLastMsg())
		return;     // eat it

	switch (nSBCode)
	{
	case SB_LINEUP:
		ScrollBy(0, -LINE_SIZE, true);
		break;

	case SB_LINEDOWN:
		ScrollBy(0, LINE_SIZE, true);
		break;

	case SB_PAGEUP:
		ScrollBy(0, -PAGE_SIZE, true);
		break;

	case SB_PAGEDOWN:
		ScrollBy(0, PAGE_SIZE, true);
		break;

	case SB_TOP:
		{
			int minPos, maxPos;
			GetScrollRange(SB_VERT, &minPos, &maxPos);
			SetScrollPos(SB_VERT, minPos);
		}
		break;

	case SB_BOTTOM:
		{
			int minPos, maxPos;
			GetScrollRange(SB_VERT, &minPos, &maxPos);
			SetScrollPos(SB_VERT, maxPos);
		}
		break;

	case SB_THUMBTRACK:
		ScrollToThumbTrackPos(true);
		break;
	}
}


BOOL CCartesianView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	CRect clientRect;
	GetClientRect(&clientRect);

	CPoint client(pt);
	ScreenToClient(&client);

	if (clientRect.PtInRect(client))
	{
		if (nFlags == 0)
		{
			if (zDelta > 0)
			{
				DoMouseZoomIn();
			}
			else
			{
				DoMouseZoomOut();
			}
		}
		else if ((nFlags & MK_CONTROL) == MK_CONTROL)
		{
			DoMouseRotate(zDelta);
		}
	}

	return CView::OnMouseWheel(nFlags, zDelta, pt);
}

BOOL CCartesianView::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}

void CCartesianView::OnInitialUpdate()
{
	// Sort out cursors.
	m_hCurCenter = AfxGetApp()->LoadCursor(IDC_CENTER);
	m_hCurZoomIn = AfxGetApp()->LoadCursor(IDC_ZOOM_IN);
	m_hCurZoomOut = AfxGetApp()->LoadCursor(IDC_ZOOM_OUT);
#if defined(ENABLE_DESIGN_MODE)
	m_hCurDefineHotspot = AfxGetApp()->LoadCursor(IDC_DEFINE_HOTSPOT);
#endif

	CSnoTrackDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	m_layers.push_back(NEW LogicalRangeLayer("Logical Boundary", 0));
	m_layers.push_back(NEW DeviceRangeLayer("Device Boundary", 10));
	m_layers.push_back(NEW GridLinesLayer(_T("Grid Lines"), 20));
	m_layers.push_back(NEW TrackLogLayer(_T("Track Log"), 40, &pDoc->m_positions));
	m_layers.push_back(NEW LiftsLayer(_T("Lifts"), 50, &pDoc->m_lifts));
	m_layers.push_back(NEW WaypointsLayer(_T("User Waypoints"), 60, &pDoc->m_user_waypoints, RGB(0,0x80,0), RGB(0xEE,0xFF,0xEE), true));
	m_layers.push_back(NEW WaypointsLayer(_T("Resort Waypoints"), 70, &pDoc->m_resort_waypoints, RGB(0x80,0x80,0), RGB(0xFF,0xFF,0xEE), false));
	m_layers.push_back(NEW ScaleLayer(_T("Scale"), 80));

#if defined(ENABLE_DESIGN_MODE)
	if (pDoc->HasHotspotGrid() && !m_pathLayer)
	{
		m_pathLayer = NEW PathLayer(_T("Paths"), 25, &pDoc->m_centres, &pDoc->m_hotspotPaths);
		m_layers.push_back(m_pathLayer);
	}
#endif

	std::stable_sort(m_layers.begin(), m_layers.end(), LayerWeightLess());

	CView::OnInitialUpdate();
}

BOOL CCartesianView::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if (nHitTest == HTCLIENT)
	{
		if (m_clickAction == ON_CLICK_CENTER)
		{
			SetCursor(m_hCurCenter);
			return TRUE;
		}
		else if (m_clickAction == ON_CLICK_ZOOM_IN)
		{
			SHORT keyState = GetAsyncKeyState(VK_CONTROL);
			if (keyState)
				SetCursor(m_hCurZoomOut);
			else
				SetCursor(m_hCurZoomIn);
			return TRUE;
		}
		else if (m_clickAction == ON_CLICK_ZOOM_OUT)
		{
			SHORT keyState = GetAsyncKeyState(VK_CONTROL);
			if (keyState)
				SetCursor(m_hCurZoomIn);
			else
				SetCursor(m_hCurZoomOut);
			return TRUE;
		}
#if defined(ENABLE_DESIGN_MODE)
		else if (m_clickAction == ON_CLICK_DEFINE_HOTSPOT)
		{
			SetCursor(m_hCurDefineHotspot);
			return TRUE;
		}
#endif
	}

	return CView::OnSetCursor(pWnd, nHitTest, message);
}

void CCartesianView::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_clickAction == ON_CLICK_CENTER)
	{
		DoMouseCenter();
	}
	else if (m_clickAction == ON_CLICK_ZOOM_IN)
	{
		SHORT keyState = GetAsyncKeyState(VK_CONTROL);
		if (keyState)
			DoMouseZoomOut();
		else
			DoMouseZoomIn();
	}
	else if (m_clickAction == ON_CLICK_ZOOM_OUT)
	{
		SHORT keyState = GetAsyncKeyState(VK_CONTROL);
		if (keyState)
			DoMouseZoomIn();
		else
			DoMouseZoomOut();
	}
#if defined(ENABLE_DESIGN_MODE)
	else if (m_clickAction == ON_CLICK_DEFINE_HOTSPOT)
	{
		DoDefineHotspot(point);
	}
#endif

	CView::OnLButtonUp(nFlags, point);
}

void CCartesianView::UpdateMouseCursor()
{
	// Not sure if this is the correct way to do this, but...
	CPoint point;
	GetCursorPos(&point);
	UINT nHitTest = (UINT)SendMessage(WM_NCHITTEST, 0, MAKELONG(point.x, point.y));
	OnSetCursor(this, nHitTest, WM_MOUSEMOVE);
}

void CCartesianView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (nChar == VK_HOME)
	{
		DoKeyboardCenter();
	}
	else if (nChar == VK_PRIOR)
	{
		DoKeyboardZoomIn();
	}
	else if (nChar == VK_NEXT)
	{
		DoKeyboardZoomOut();
	}
	else if (nChar == 'Q')
	{
		DoKeyboardRotateCCW();
	}
	else if (nChar == 'W')
	{
		DoKeyboardRotateCW();
	}
	else if (nChar == VK_CONTROL)
	{
		UpdateMouseCursor();
	}

	CView::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CCartesianView::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (nChar == VK_CONTROL)
	{
		UpdateMouseCursor();
	}

	CView::OnKeyUp(nChar, nRepCnt, nFlags);
}

/** Get the current mouse position in client coordinates.
 * Return true if the mouse is in the client area, false otherwise.
 */
bool CCartesianView::GetClientCursorPos(CPoint *point)
{
	GetCursorPos(point);
	ScreenToClient(point);

	CRect clientRect;
	GetClientRect(&clientRect);

	return (clientRect.PtInRect(*point) != FALSE);
}

void CCartesianView::DoMouseCenter()
{
	CPoint point;
	if (GetClientCursorPos(&point))
	{
		// Scroll the display by how far the mouse is from the center of the
		// client area -- no dicking about with projections required.
		CRect clientRect;
		GetClientRect(clientRect);

		ScrollBy(point - clientRect.CenterPoint(), true);
	}
}

bool CCartesianView::CanZoomIn() const
{
	return (m_scaleFactor > 1);
}

void CCartesianView::ZoomIn()
{
	if (CanZoomIn())
		m_scaleFactor /= 2;
}

void CCartesianView::DoMouseZoomIn()
{
	CPoint point;
	if (GetClientCursorPos(&point) && CanZoomIn())
		DoZoomIn(point);
}

bool CCartesianView::CanZoomOut() const
{
	return (m_scaleFactor < MAX_SCALE_FACTOR);
}

void CCartesianView::ZoomOut()
{
	if (CanZoomOut())
		m_scaleFactor *= 2;
}

void CCartesianView::DoZoomIn(CPoint ptClient)
{
	CPoint r(GetWindowPointFromClientPoint(ptClient));

	ZoomIn();

	// If I zoom in, then the device range will have changed, but the logical range and the window range will still be the same.
	RecalcDeviceRange();
	UpdateScrollBars();

	ScrollToWindowPoint(r, ptClient);
}

void CCartesianView::DoZoomOut(CPoint ptClient)
{
	CPoint r(GetWindowPointFromClientPoint(ptClient));

	ZoomOut();

	// If I zoom out, then the device range will have changed, but the logical range and the window range will still be the same.
	RecalcDeviceRange();
	UpdateScrollBars();

	ScrollToWindowPoint(r, ptClient);
}

void CCartesianView::DoMouseZoomOut()
{
	CPoint point;
	if (GetClientCursorPos(&point) && CanZoomOut())
		DoZoomOut(point);
}

/** If dTheta is +ve, then we rotate anti-clockwise. */
void CCartesianView::DoRotate(double dTheta, CPoint ptClient)
{
	CPoint r(GetLogicalPointFromClientPoint(ptClient));

	m_rotation.theta += dTheta;
	m_rotation.sin_theta = sin(m_rotation.theta);
	m_rotation.cos_theta = cos(m_rotation.theta);

	TRACE("Rotating view to %f degrees.\n", RadiansToDegrees(m_rotation.theta));

	// If I rotate, then the window range will have changed, as will the device range.
	RecalcWindowRange();
	RecalcDeviceRange();
	UpdateScrollBars();

	ScrollToLogicalPoint(r, ptClient);
}

void CCartesianView::DoMouseRotate(short zDelta)
{
	CPoint point;
	if (GetClientCursorPos(&point))
		DoRotate(-M_PI_18 * zDelta / 120, point);
}

void CCartesianView::DoKeyboardCenter()
{
	DoMouseCenter();
}

void CCartesianView::DoKeyboardZoomIn()
{
	DoMouseZoomIn();
}

void CCartesianView::DoKeyboardZoomOut()
{
	DoMouseZoomOut();
}

void CCartesianView::DoKeyboardRotateCCW()
{
	CPoint point;
	if (GetClientCursorPos(&point))
		DoRotate(M_PI_18, point);
}

void CCartesianView::DoKeyboardRotateCW()
{
	CPoint point;
	if (GetClientCursorPos(&point))
		DoRotate(-M_PI_18, point);
}

void CCartesianView::OnViewZoomin()
{
	CRect clientRect;
	GetClientRect(&clientRect);

	DoZoomIn(clientRect.CenterPoint());
}

void CCartesianView::OnViewZoomout()
{
	CRect clientRect;
	GetClientRect(&clientRect);

	DoZoomOut(clientRect.CenterPoint());
}

void CCartesianView::OnUpdateViewZoomin(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(CanZoomIn());
}

void CCartesianView::OnUpdateViewZoomout(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(CanZoomOut());
}

void CCartesianView::OnViewRotateccw()
{
	CRect clientRect;
	GetClientRect(&clientRect);

	DoRotate(M_PI_18, clientRect.CenterPoint());
}

void CCartesianView::OnViewRotatecw()
{
	CRect clientRect;
	GetClientRect(&clientRect);

	DoRotate(-M_PI_18, clientRect.CenterPoint());
}

void CCartesianView::OnModeZoomin()
{
	m_clickAction = ON_CLICK_ZOOM_IN;

	// Also, Windows XP Image Viewer -style, do a single instance of the action.
	OnViewZoomin();
}

void CCartesianView::OnModeZoomout()
{
	m_clickAction = ON_CLICK_ZOOM_OUT;

	// Also, Windows XP Image Viewer -style, do a single instance of the action.
	OnViewZoomout();
}

void CCartesianView::OnModeCenter()
{
	m_clickAction = ON_CLICK_CENTER;
}

void CCartesianView::OnUpdateModeZoomin(CCmdUI *pCmdUI)
{
	pCmdUI->SetRadio(m_clickAction == ON_CLICK_ZOOM_IN);
}

void CCartesianView::OnUpdateModeZoomout(CCmdUI *pCmdUI)
{
	pCmdUI->SetRadio(m_clickAction == ON_CLICK_ZOOM_OUT);
}

void CCartesianView::OnUpdateModeCenter(CCmdUI *pCmdUI)
{
	pCmdUI->SetRadio(m_clickAction == ON_CLICK_CENTER);
}

void CCartesianView::OnUpdateLongitude(CCmdUI *pCmdUI)
{
	CPoint point;
	if (GetClientCursorPos(&point))
	{
		// 'point' is in client coordinates.  Map it back into projection coordinates.
		CPoint p = GetLogicalPointFromClientPoint(point);

		// Run that back through the projection.
		char *pargs[] = {
			"proj=merc", "ellps=WGS84",
		};
		Projection proj;
		VERIFY(proj.Create(COUNTOF(pargs), pargs));
		projUV out = proj.ReverseDeg(p);

		CString lon = FormatLongitude(out.u, FORMAT_LON_GARMIN);

		pCmdUI->Enable(TRUE);
		pCmdUI->SetText(lon);
	}
	else
		pCmdUI->Enable(FALSE);
}

void CCartesianView::OnUpdateLatitude(CCmdUI *pCmdUI)
{
	CPoint point;
	if (GetClientCursorPos(&point))
	{
		// 'point' is in client coordinates.  Map it back into projection coordinates.
		CPoint p = GetLogicalPointFromClientPoint(point);

		// Run that back through the projection.
		char *pargs[] = {
			"proj=merc", "ellps=WGS84",
		};
		Projection proj;
		VERIFY(proj.Create(COUNTOF(pargs), pargs));
		projUV out = proj.ReverseDeg(p);

		CString lat = FormatLatitude(out.v, FORMAT_LAT_GARMIN);

		pCmdUI->Enable(TRUE);
		pCmdUI->SetText(lat);
	}
	else
        pCmdUI->Enable(FALSE);
}

void CCartesianView::OnUpdateScale(CCmdUI *pCmdUI)
{
	CDC *pDC = GetDC();
	int xDpi = pDC->GetDeviceCaps(LOGPIXELSX);
	ReleaseDC(pDC);

	// LOGPIXELSX gives us the logical dpi of the screen.  The factor of 10 is
	// because our pixels (depending on the scale) actually measure 10cm units
	// (or 10/2.54 inches).
	int approx_scale = m_scaleFactor * (int)floor(0.5 + (10 * xDpi / 2.54));

	CString str;
	str.Format(_T("1:%d"), approx_scale);

	pCmdUI->Enable(TRUE);
	pCmdUI->SetText(str);
}

void CCartesianView::OnMouseMove(UINT nFlags, CPoint point)
{
#if 0 && defined(_DEBUG)
	CSnoTrackDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// Find the point closest to the mouse.  This is O(n) in this implementation:
	CPoint p = GetLogicalPointFromClientPoint(point);

	PositionFixCollection::const_iterator closest = pDoc->m_positions.end();
	double closest_dist = DBL_MAX;
	for (PositionFixCollection::const_iterator i = pDoc->m_positions.begin(); i != pDoc->m_positions.end(); ++i)
	{
		// The logical point is in projected units (i.e. 10cm units), so we can measure distance using it.
		int px = i->GetProjectionX();
		int py = i->GetProjectionY();
		int dx = p.x - px;
		int dy = p.y - py;

		double dist = hypot(dx, dy);
		ASSERT(dist >= 0);
		//TRACE("px = %d, py = %d, dx = %d, dy = %d, dist = %f\n", px, py, dx, dy, dist);

		if (dist < closest_dist)
		{
			closest_dist = dist;
			closest = i;
		}
	}

	if (closest != pDoc->m_positions.end())
	{
		TRACE("Closest point: %d, %f, %f (%f)\n", closest->GetUnixTimestamp(), closest->GetLongitude(), closest->GetLatitude(), closest_dist);
	}
#endif

	CView::OnMouseMove(nFlags, point);
}

void CCartesianView::OnPaint()
{
	CPaintDC dc(this);

	// Don't OnPrepareDC(&dc);
	OnDraw(&dc, &dc.m_ps);
}

void CCartesianView::OnViewOptions()
{
	CViewOptionsDialog dlg(&m_layers);

	if (dlg.DoModal() == IDOK)
	{
		NotifyRecalcLayers();
		Invalidate();
	}
}

int CCartesianView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	return 0;
}

void CCartesianView::OnDestroy()
{
	for (Layers::iterator i = m_layers.begin(); i != m_layers.end(); ++i)
	{
		Layer *layer = *i;
		delete layer;
	}
	m_layers.clear();

	CView::OnDestroy();
}

#if defined(ENABLE_DESIGN_MODE)
void CCartesianView::OnHotspotsDefinehotspots()
{
	CSnoTrackDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	if (m_clickAction == ON_CLICK_DEFINE_HOTSPOT)
	{
		// Stop defining hotspots.
		ASSERT(m_hotspotLayer != NULL);

		m_clickAction = m_prevClickAction;
		m_hotspotLayer->Enable(false);

		Invalidate();

		pDoc->RecalcRoute();
		pDoc->UpdateAllViews(this, HINT_EDIT_RESORT_HOTSPOTS);
	}
	else
	{
		// Start defining hotspots.
		if (m_hotspotLayer)
		{
			// We've already started, so start again.
			ASSERT(pDoc->m_hotspotGrid != NULL);

			m_hotspotLayer->Enable(true);

			m_prevClickAction = m_clickAction;
			m_clickAction = ON_CLICK_DEFINE_HOTSPOT;

			Invalidate();
		}
		else
		{
			if (pDoc->HasHotspotGrid() || pDoc->CreateHotspotGrid())
			{
				m_hotspotLayer = NEW HotspotGridLayer(_T("Hotspot Grid"), 30, pDoc->m_hotspotGrid);
				m_layers.push_back(m_hotspotLayer);
				m_pathLayer = NEW PathLayer(_T("Paths"), 25, &pDoc->m_centres, &pDoc->m_hotspotPaths);
				m_layers.push_back(m_pathLayer);
				std::stable_sort(m_layers.begin(), m_layers.end(), LayerWeightLess());

				m_hotspotLayer->Enable(true);

				m_prevClickAction = m_clickAction;
				m_clickAction = ON_CLICK_DEFINE_HOTSPOT;

				Invalidate();
			}
		}
	}
}

void CCartesianView::OnUpdateHotspotsDefinehotspots(CCmdUI *pCmdUI)
{
	BOOL bCheck = (m_clickAction == ON_CLICK_DEFINE_HOTSPOT);
	pCmdUI->SetCheck(bCheck);
}

void CCartesianView::OnHotspotsClearhotspots()
{
	UINT nResult = AfxMessageBox("This will remove all hotspots.\nAre you sure?", MB_YESNO|MB_DEFBUTTON2);
	if (nResult != IDYES)
		return;

	// If we're displaying the hotspot layer, then lose it.
	if (m_hotspotLayer != NULL)
	{
		Layers::iterator i = std::find(m_layers.begin(), m_layers.end(), m_hotspotLayer);
		ASSERT(i != m_layers.end());
		m_layers.erase(i);
		std::stable_sort(m_layers.begin(), m_layers.end(), LayerWeightLess());

		delete m_hotspotLayer;
		m_hotspotLayer = NULL;

		m_clickAction = m_prevClickAction;
	}

	// Now lose the grid.
	CSnoTrackDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	pDoc->DeleteHotspotGrid();
	Invalidate();

	pDoc->RecalcRoute();
	pDoc->UpdateAllViews(this, HINT_EDIT_RESORT_HOTSPOTS);
}

void CCartesianView::DoDefineHotspot(CPoint point)
{
	ASSERT(m_hotspotLayer != NULL);

	CSnoTrackDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	CPoint logicalPoint = GetLogicalPointFromClientPoint(point);
	pDoc->ToggleHotspot(logicalPoint, m_hotspotSize);
	Invalidate();
}

class HidePositionFix
{
	bool m_hide;

public:
	HidePositionFix(bool hide)
		: m_hide(hide)
	{
	}

	void operator() (PositionFix &positionFix)
	{
		positionFix.SetHidden(m_hide);
	}
};

void CCartesianView::OnResortHidelifts()
{
	CSnoTrackDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	CWaitCursor wait;
	DWORD startTime = GetTickCount();

	const RouteEventCollection &route = pDoc->GetRoute();
	
	// Convert the route events into a selection.
	SelectionTimeSpanCollection selection;
	selection.reserve(route.size());
	for (RouteEventCollection::const_iterator i = route.begin(); i != route.end(); ++i)
	{
		const HotspotPath *path = i->path;
		if (path && path->IsLift())
			selection.push_back(SelectionTimeSpan(i->startTime, i->endTime));
	}

	// Ensure that the selections are sorted:
	std::sort(selection.begin(), selection.end(), SelectionTimeSpanLess());
	SelectionTimeSpanCollection::const_iterator s = selection.begin();
	
	// Iterate over the position fixes, highlighting them.
	PositionFixCollection::iterator p_begin = pDoc->m_positions.begin();
	PositionFixCollection::iterator p_end = pDoc->m_positions.end();
	CorrelatePositionFixes(p_begin, p_end, selection.begin(), selection.end(), HidePositionFix(true), HidePositionFix(false));

	NotifySelectLayers();

	// TODO: Try to avoid doing this by working out which regions got deselected.
	Invalidate();

	DWORD endTime = GetTickCount();
	TRACE("OnResortHidelifts took %d ms.\n", endTime - startTime);
}

void CCartesianView::OnResortHidematchedpaths()
{
	CSnoTrackDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	CWaitCursor wait;
	DWORD startTime = GetTickCount();

	const RouteEventCollection &route = pDoc->GetRoute();
	
	// Convert the route events into a selection.
	SelectionTimeSpanCollection selection;
	selection.reserve(route.size());
	for (RouteEventCollection::const_iterator i = route.begin(); i != route.end(); ++i)
	{
		selection.push_back(SelectionTimeSpan(i->startTime, i->endTime));
	}

	// Ensure that the selections are sorted:
	std::sort(selection.begin(), selection.end(), SelectionTimeSpanLess());
	SelectionTimeSpanCollection::const_iterator s = selection.begin();
	
	// Iterate over the position fixes, highlighting them.
	PositionFixCollection::iterator p_begin = pDoc->m_positions.begin();
	PositionFixCollection::iterator p_end = pDoc->m_positions.end();
	CorrelatePositionFixes(p_begin, p_end, selection.begin(), selection.end(), HidePositionFix(true), HidePositionFix(false));

	NotifySelectLayers();

	// TODO: Try to avoid doing this by working out which regions got deselected.
	Invalidate();

	DWORD endTime = GetTickCount();
	TRACE("OnResortHidematchedpaths took %d ms.\n", endTime - startTime);
}

void CCartesianView::OnResortShowall()
{
	CSnoTrackDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	CWaitCursor wait;
	DWORD startTime = GetTickCount();

	// Iterate over the position fixes, highlighting them.
	PositionFixCollection::iterator b = pDoc->m_positions.begin();
	PositionFixCollection::iterator e = pDoc->m_positions.end();
	for (PositionFixCollection::iterator i = b; i != e; ++i)
	{
		i->SetHidden(false);
	}

	NotifySelectLayers();

	// TODO: Try to avoid doing this by working out which regions got deselected.
	Invalidate();

	DWORD endTime = GetTickCount();
	TRACE("OnResortHidematchedpaths took %d ms.\n", endTime - startTime);
}

void CCartesianView::OnResortHotspotsize()
{
	CHotspotSizeDialog dlg(m_hotspotSize);
	if (dlg.DoModal() == IDOK)
		m_hotspotSize = dlg.GetHotspotSize();
}
#endif /* ENABLE_DESIGN_MODE */
