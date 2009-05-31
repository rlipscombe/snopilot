#pragma once

#include "PositionFix.h"
#include "Waypoint.h"
#include "Lift.h"
#include "Layer.h"
#include "Rotation.h"
#include "SelectionTimeSpan.h"

class CSnoTrackDoc;
class CTimeLineWnd;
class Projection;
struct GridBounds;

class HotspotGridLayer;
class PathLayer;

// CCartesianView view
class CCartesianView : public CView
{
	HCURSOR m_hCurCenter;
	HCURSOR m_hCurZoomIn;
	HCURSOR m_hCurZoomOut;
#if defined(ENABLE_DESIGN_MODE)
	HCURSOR m_hCurDefineHotspot;
#endif

	enum ClickAction {
		ON_CLICK_NONE = -1,
		ON_CLICK_CENTER = 0,
		ON_CLICK_ZOOM_IN = 1,
		ON_CLICK_ZOOM_OUT = 2,
#if defined(ENABLE_DESIGN_MODE)
		ON_CLICK_DEFINE_HOTSPOT = 3,
#endif
	};
	
	ClickAction m_clickAction;
	ClickAction m_prevClickAction;

	Layers m_layers;
	SelectionTimeSpanCollection m_selection;

#if defined(ENABLE_DESIGN_MODE)
	HotspotGridLayer *m_hotspotLayer;
	PathLayer *m_pathLayer;
	int m_hotspotSize;
#endif

	DECLARE_DYNCREATE(CCartesianView)

protected:
	CCartesianView();
	virtual ~CCartesianView();

public:
	CSnoTrackDoc* GetDocument() const;

	virtual void OnDraw(CDC *pDC) { OnDraw(pDC, NULL); }
	void OnDraw(CDC* pDC, const PAINTSTRUCT *ps);
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

public:
	void SelectTimeSpan(const SelectionTimeSpan &span);
	void SelectTimeSpans(const SelectionTimeSpanCollection &selection);

private:
	void HighlightPositionFixes(CRect *pRange);

protected:
	DECLARE_MESSAGE_MAP()
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnInitialUpdate();
	virtual void OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/);

	void DoDraw(CDC *pDC, const PAINTSTRUCT *ps);
	void NotifyRecalcLayers();
	void NotifySelectLayers();

	friend class CartesianViewDrawSite;	// for GetClientPointFromLogicalPoint

// Scrolling.
private:
	void ScrollToPos(int xPos, int yPos, bool bScrollWindow);
	inline void ScrollToPos(CPoint pt, bool bScrollWindow) { ScrollToPos(pt.x, pt.y, bScrollWindow); }
	void ScrollBy(int xScroll, int yScroll, bool bScrollWindow);
	inline void ScrollBy(SIZE size, bool bScrollWindow) { ScrollBy(size.cx, size.cy, bScrollWindow); }

	int GetTrackPos(int nBar);
	void ScrollToThumbTrackPos(bool bScrollWindow);

	void ScrollToCenter();

	CRect CalculateScrollRange();
	void UpdateScrollBars();

	CPoint GetScrollPos();
	using CView::GetScrollPos;

// Coordinate mappings
public:
	/* Coordinate mappings:
	 *
	 * This program deals with 5 coordinate spaces.  It's more complicated
	 * than it should be (we should look at using world transforms), but it
	 * works.
	 *
	 * The spaces are as follows:
	 *
	 *  World coordinates (polar: lat/lon pairs)
	 *           |
	 *      (projection)
	 *           |
	 *           V
	 *  Logical coordinates (cartesian: 10cm units)
	 *           |
	 *       (rotation)
	 *           |
	 *           V
	 *  Window coordinates
	 *           |
	 *       (scaling)
	 *           |
	 *           V
	 *  Device coordinates (pixels)
	 *           |
	 *       (scrolling)
	 *           |
	 *           V
	 *  Client coordinates (pixels)
	 */
// TODO: It'd be nice to put these in a separate object.  The problem is that
// the device <-> client mapping requires access to GetScrollPos.
	void RecalcAllRanges();
	void RecalcDeviceRange();
	void RecalcWindowRange();

	CRect GetLogicalRange() const { return m_logicalRange; }
	CRect GetWindowRange() const { return m_windowRange; }
	CRect GetDeviceRange() const { return m_deviceRange; }

	CRect m_logicalRange;	///< Recalculated when points are added/removed.
	CRect m_windowRange;	///< Depends on rotation.
	CRect m_deviceRange;	///< Depends on scale, but is an easy one to work out.

	// client <- device <- window <- logical
	CPoint GetClientPointFromDevicePoint(CPoint device);
	CPoint GetDevicePointFromWindowPoint(CPoint window);
	CPoint GetWindowPointFromLogicalPoint(CPoint logical);

	CPoint GetClientPointFromLogicalPoint(CPoint logical);
	CPoint GetDevicePointFromLogicalPoint(CPoint logical);

	// logical <- window <- device <- client
	CPoint GetLogicalPointFromWindowPoint(CPoint window);
	CPoint GetWindowPointFromDevicePoint(CPoint device);
	CPoint GetDevicePointFromClientPoint(CPoint client);

	CPoint GetWindowPointFromClientPoint(CPoint client);
	CPoint GetLogicalPointFromClientPoint(CPoint client);

	CRect GetLogicalRangeFromGridBounds(const GridBounds &gridBounds) const;

private:
	int m_scaleFactor;
	Rotation m_rotation;

private:
	void ScrollToLogicalPoint(CPoint logicalPoint, CPoint clientPoint);
	void ScrollToWindowPoint(CPoint windowPoint, CPoint clientPoint);
	void ScrollToDevicePoint(CPoint oldDevicePoint, CPoint newDevicePoint);
	void CenterOnWindowPoint(CPoint pt);
	void CenterOnWindowRange(const CRect &windowRange);

private:
	bool GetClientCursorPos(CPoint *point);
	void UpdateMouseCursor();

	bool CanZoomIn() const;
	void ZoomIn();

	bool CanZoomOut() const;
	void ZoomOut();

	void DoDefineHotspot(CPoint point);

	void DoMouseCenter();
	void DoMouseZoomOut();
	void DoMouseZoomIn();
	void DoMouseRotate(short zDelta);

	void DoKeyboardCenter();
	void DoKeyboardZoomIn();
	void DoKeyboardZoomOut();
	void DoKeyboardRotateCCW();
	void DoKeyboardRotateCW();

	void DoZoomIn(CPoint ptClient);
	void DoZoomOut(CPoint ptClient);
	void DoRotate(double dTheta, CPoint ptClient);

	void SetDefaultViewProperties();

public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnUpdateLongitude(CCmdUI *pCmdUI);
	afx_msg void OnUpdateLatitude(CCmdUI *pCmdUI);
	afx_msg void OnUpdateScale(CCmdUI *pCmdUI);
	afx_msg void OnViewZoomin();
	afx_msg void OnUpdateViewZoomin(CCmdUI *pCmdUI);
	afx_msg void OnViewZoomout();
	afx_msg void OnUpdateViewZoomout(CCmdUI *pCmdUI);
	afx_msg void OnViewRotateccw();
	afx_msg void OnViewRotatecw();
	afx_msg void OnModeZoomin();
	afx_msg void OnUpdateModeZoomin(CCmdUI *pCmdUI);
	afx_msg void OnModeZoomout();
	afx_msg void OnUpdateModeZoomout(CCmdUI *pCmdUI);
	afx_msg void OnModeCenter();
	afx_msg void OnUpdateModeCenter(CCmdUI *pCmdUI);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnViewOptions();

#if defined(ENABLE_DESIGN_MODE)
	afx_msg void OnHotspotsDefinehotspots();
	afx_msg void OnUpdateHotspotsDefinehotspots(CCmdUI *pCmdUI);
	afx_msg void OnHotspotsClearhotspots();
#endif
	afx_msg void OnResortHidelifts();
	afx_msg void OnResortHidematchedpaths();
	afx_msg void OnResortShowall();
	afx_msg void OnResortHotspotsize();
};

#ifndef _DEBUG  // debug version in SnoTrackView.cpp
inline CSnoTrackDoc* CCartesianView::GetDocument() const
{
	return reinterpret_cast<CSnoTrackDoc*>(m_pDocument);
}
#endif

