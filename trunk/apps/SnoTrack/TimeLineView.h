#pragma once
#include "win32_ui/DialogToolBar.h"
#include "TimeLineWnd.h"
#include "SnoTrackDoc.h"

// CTimeLineView view

class CTimeLineView : public CView
{
	CDialogToolBar m_wndToolBar;
	CTimeLineWnd m_wndTimeLine;

	DECLARE_DYNCREATE(CTimeLineView)

protected:
	CTimeLineView();           // protected constructor used by dynamic creation
	virtual ~CTimeLineView();

public:
	CSnoTrackDoc* GetDocument() const;

	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);

protected:
	void RecalcLayout();

	virtual void OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

	DECLARE_MESSAGE_MAP()

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnViewZoomin();
	afx_msg void OnViewZoomout();
	afx_msg void OnUpdateViewZoomout(CCmdUI *pCmdUI);
	afx_msg void OnUpdateViewZoomin(CCmdUI *pCmdUI);
	afx_msg void OnViewHide();
	afx_msg void OnViewUnhide();
};

#ifndef _DEBUG  // debug version in SnoTrackView.cpp
inline CSnoTrackDoc* CTimeLineView::GetDocument() const
   { return reinterpret_cast<CSnoTrackDoc*>(m_pDocument); }
#endif
