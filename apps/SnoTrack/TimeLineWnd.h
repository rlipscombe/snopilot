#pragma once
#include "PositionFix.h"
#include "win32/filetime_helpers.h"
#include "win32_ui/TrackingToolTip.h"
#include <vector>
#include "CalcRoute.h"

class CTimeLineWnd : public CWnd
{
	const PositionFixCollection *m_positionFixes;
	TIME_ZONE_INFORMATION m_tzi;

	CImageList m_imageList;

	// Since, when drawing, the y axis increases downwards, we figure coordinates out relative to the max elevation.
	double m_maxElevation;
	double m_rangeElevation;

	RouteEventCollection m_route;

	// For tooltips in the footer.
	typedef std::vector< CRect> HitRectCollection;
	HitRectCollection m_hitRects;
	int m_lastHitTest;

	class TimeSpan
	{
		FILETIME m_startTime;
		FILETIME m_endTime;

		bool m_bVisible;		///< Is it visible at the moment?

	public:
		TimeSpan(FILETIME startTime, FILETIME endTime, bool bVisible)
			: m_startTime(startTime), m_endTime(endTime), m_bVisible(bVisible)
		{
			ASSERT(m_startTime <= m_endTime);
		}

		FILETIME GetStartTime() const { return m_startTime; }
		FILETIME GetEndTime() const { return m_endTime; }

		bool IsVisible() const { return m_bVisible; }
		void SetVisible(bool b = true) { m_bVisible = b; }
	};

	typedef std::vector< TimeSpan > TimeSpanCollection;
	TimeSpanCollection m_timeSpans;

	static int m_zoomLevels[];
	static int m_zoomLevelCount;
	int m_zoomIndex;
	static const int DEFAULT_ZOOM_INDEX = 2;

	CFont m_scaleFont;
	int m_scaleHeight;
	int m_footerHeight;

	CPoint m_lastMousePosition;
	BOOL m_bTrackLeave;		///< Used with TrackMouseEvent, so that we can hide the tooltip when the mouse leaves the window.

	// Selection stuff
	FILETIME m_selectionAnchor;
	FILETIME m_selectionPoint;
	CRect m_selectionRect;

	DECLARE_DYNAMIC(CTimeLineWnd)

public:
	CTimeLineWnd();

	BOOL Create(CWnd *pParentWnd, UINT nID);

	void UpdateTimeLine(LPCTSTR lpszTimeZoneName, FILETIME minTimestamp, FILETIME maxTimestamp,
						const PositionFixCollection *positionFixes, double minElevation, double maxElevation,
						const RouteEventCollection &route);

// Zooming stuff
public:
	void ZoomIn();
	bool CanZoomIn() const;
	void ZoomOut();
	bool CanZoomOut() const;

	void SetZoomIndex(int zoomIndex);

public:
	void HideSpans();
	void ShowAllSpans();

// Scrolling stuff
private:
	void ScrollBy(int xScroll, int yScroll, bool bScrollWindow);

	void ScrollToThumbTrackPos(bool bScrollWindow);
	int GetTrackPos(int nBar);

	CPoint GetScrollPos();
	using CWnd::GetScrollPos;

	void ScrollToPos(int xPos, int yPos, bool bScrollWindow);
	inline void ScrollToPos(CPoint pt, bool bScrollWindow) { ScrollToPos(pt.x, pt.y, bScrollWindow); }

	void UpdateScrollBar(BOOL bRedraw);

	// Ullage at the right and left ends of the timeline.
	static const int SCROLL_MARGIN = 15;

private:
	void RecalcSpans();
	void DumpTimeSpans() const;

	int CalculateSpanWidthInPixels(const TimeSpan &timeSpan) const;

private:
	int ConvertSecondsToPixels(ULONGLONG seconds) const;
	ULONGLONG ConvertPixelsToSeconds(int pixels) const;
	int GetTickIntervalInSeconds() const;

// Drawing stuff
private:
	void DoPaint(CDC *pDC);
	
	void DrawLeftMargin(CDC *pDC, const CRect &clientRect, const CPoint &scrollPos) const;
	void DrawRightMargin(CDC *pDC, const CRect &rect) const;

	void DrawSpans(CDC *pDC, const CRect &clientRect, const CPoint &scrollPos, RECT *pRemaining);
	void DrawSpan(CDC *pDC, const CRect &spanRect, const TimeSpan &timeSpan);
	void DrawVisibleSpan(CDC *pDC, const CRect &spanRect, const TimeSpan &timeSpan);
	void DrawVisibleSpanScale(CDC *pDC, const CRect &scaleRect, const TimeSpan &timeSpan) const;
	void DrawVisibleSpanBody(CDC *pDC, const CRect &bodyRect, const TimeSpan &timeSpan) const;
	void DrawVisibleSpanFooter(CDC *pDC, const CRect &footerRect, const TimeSpan &timeSpan);

	void DrawHiddenSpan(CDC *pDC, const CRect &spanRect, const TimeSpan &timeSpan) const;

	CPoint CalculatePixelPosition(const CRect &rect, FILETIME spanStart, FILETIME thisTimestamp, double thisElevation) const;
	int GetTimestampDrawThreshold() const;

	FILETIME CalculateNextDrawableTimestamp(FILETIME thisTimestamp) const;

	PositionFixCollection::const_iterator
		FindNextDrawablePositionFix(PositionFixCollection::const_iterator i, PositionFixCollection::const_iterator e) const;

	CRect CalculateEventRect(const CRect &rect, FILETIME spanStart, FILETIME startTime, FILETIME endTime) const;

	int CalculateScaleHeight();
	int GetScaleHeight() const { return m_scaleHeight; }
	int GetFooterHeight() const { return m_footerHeight; }

	int CalculateTickPosition(const CRect &rect, FILETIME spanStart, FILETIME thisTimestamp) const;
	CString GetTimestampText(FILETIME tickTime) const;

	BOOL HitTestSpans(CPoint point, CRect *pSpanRect, FILETIME *pFileTime);
	CRect GetTimeSpanRect(FILETIME startTime, FILETIME endTime);
	void UpdateSelectionRect();

// Tooltip stuff
private:
	CTrackingToolTip m_timeTip;
	void UpdateTrackingToolTip(CPoint point);

	void DrawVerticalHighlight(CPoint point);
	void DeleteVerticalHighlight();

protected:
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg LRESULT OnMouseLeave(WPARAM wParam, LPARAM lParam);
	afx_msg void OnExitMenuLoop(BOOL bIsTrackPopupMenu);

	DECLARE_MESSAGE_MAP()
};

struct RouteEventLessByStartTime
{
	bool operator() (const RouteEvent &lhs, const RouteEvent &rhs) const
	{
		return (CompareFileTime(&lhs.startTime, &rhs.startTime) < 0);
	}

	bool operator() (const RouteEvent &lhs, const FILETIME &rhs) const
	{
		return (CompareFileTime(&lhs.startTime, &rhs) < 0);
	}

	bool operator() (const FILETIME &lhs, const RouteEvent &rhs) const
	{
		return (CompareFileTime(&lhs, &rhs.startTime) < 0);
	}
};

struct RouteEventLessByEndTime
{
	bool operator() (const RouteEvent &lhs, const RouteEvent &rhs) const
	{
		return (CompareFileTime(&lhs.endTime, &rhs.endTime) < 0);
	}

	bool operator() (const FILETIME &lhs, const RouteEvent &rhs) const
	{
		return (CompareFileTime(&lhs, &rhs.endTime) < 0);
	}

	bool operator() (const RouteEvent &lhs, const FILETIME &rhs) const
	{
		return (CompareFileTime(&lhs.endTime, &rhs) < 0);
	}

};
