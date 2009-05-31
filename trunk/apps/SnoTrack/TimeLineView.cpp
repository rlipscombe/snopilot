// TimeLineView.cpp : implementation file
//

#include "stdafx.h"
#include "TimeLineView.h"
#include "resource.h"
#include "win32_ui/ToolBar32.h"
#include "CalcRoute.h"
#include "ChildFrm.h"
#include "CartesianView.h"
#include ".\timelineview.h"

// CTimeLineView

IMPLEMENT_DYNCREATE(CTimeLineView, CView)

CTimeLineView::CTimeLineView()
{
}

CTimeLineView::~CTimeLineView()
{
}

BEGIN_MESSAGE_MAP(CTimeLineView, CView)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_COMMAND(ID_VIEW_ZOOMIN, OnViewZoomin)
	ON_COMMAND(ID_VIEW_ZOOMOUT, OnViewZoomout)
	ON_UPDATE_COMMAND_UI(ID_VIEW_ZOOMOUT, OnUpdateViewZoomout)
	ON_UPDATE_COMMAND_UI(ID_VIEW_ZOOMIN, OnUpdateViewZoomin)
	ON_WM_DESTROY()
	ON_COMMAND(ID_VIEW_HIDE, OnViewHide)
	ON_COMMAND(ID_VIEW_UNHIDE, OnViewUnhide)
END_MESSAGE_MAP()

// CTimeLineView drawing

void CTimeLineView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}


// CTimeLineView diagnostics

#ifdef _DEBUG
void CTimeLineView::AssertValid() const
{
	CView::AssertValid();
}

void CTimeLineView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CSnoTrackDoc* CTimeLineView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CSnoTrackDoc)));
	return (CSnoTrackDoc*)m_pDocument;
}
#endif //_DEBUG


// CTimeLineView message handlers

int CTimeLineView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT | TBSTYLE_TRANSPARENT) ||
		!m_wndToolBar.LoadToolBar(IDR_TIMELINE))
	{
		TRACE("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	Load32BitColorToolBar(&m_wndToolBar, IDB_TIMELINE32);

	if (!m_wndTimeLine.Create(this, -1))
		return -1;

	return 0;
}

void CTimeLineView::RecalcLayout()
{
	CRect clientRect;
	GetClientRect(&clientRect);

	CRect remaining;
	RepositionBars(0, 0xffff, AFX_IDW_PANE_FIRST, reposQuery, &remaining, &clientRect);
	RepositionBars(0, 0xffff, AFX_IDW_PANE_FIRST, reposDefault, NULL, &clientRect);

	m_wndTimeLine.SetWindowPos(NULL, remaining.left, remaining.top, remaining.Width(), remaining.Height(), SWP_NOZORDER);
}

void CTimeLineView::OnSize(UINT nType, int cx, int cy)
{
	RecalcLayout();
}

void CTimeLineView::OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/)
{
	CSnoTrackDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// Update the timeline window.
	if (!pDoc->IsEmptyDataSet())
	{
		m_wndTimeLine.UpdateTimeLine(pDoc->GetTimeZoneName().c_str(), pDoc->GetMinimumTimestamp(), pDoc->GetMaximumTimestamp(),
			&pDoc->m_positions, pDoc->GetMinimumElevation(), pDoc->GetMaximumElevation(), pDoc->GetRoute());
	}
}

BOOL CTimeLineView::PreCreateWindow(CREATESTRUCT& cs)
{
	if (!CView::PreCreateWindow(cs))
		return FALSE;

	cs.style &= ~WS_BORDER;
	cs.dwExStyle &= ~WS_EX_CLIENTEDGE;

	return TRUE;
}

void CTimeLineView::OnViewZoomin()
{
	m_wndTimeLine.ZoomIn();
}

void CTimeLineView::OnUpdateViewZoomin(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_wndTimeLine.CanZoomIn());
}

void CTimeLineView::OnViewZoomout()
{
	m_wndTimeLine.ZoomOut();
}

void CTimeLineView::OnUpdateViewZoomout(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_wndTimeLine.CanZoomOut());
}

void CTimeLineView::OnDestroy()
{
	delete m_wndToolBar.GetToolBarCtrl().GetImageList();
	delete m_wndToolBar.GetToolBarCtrl().GetHotImageList();
	delete m_wndToolBar.GetToolBarCtrl().GetDisabledImageList();

	CView::OnDestroy();
}

BOOL CTimeLineView::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	if (nCode == CN_COMMAND)
	{
		// All other commands should be sent to the cartesian view.
		if (nID != ID_VIEW_ZOOMIN && nID != ID_VIEW_ZOOMOUT && nID != ID_VIEW_HIDE && nID != ID_VIEW_UNHIDE)
		{
			CChildFrame *pFrame = static_cast<CChildFrame *>(GetParentFrame());
			CCartesianView *pView = pFrame->GetCartesianView();
			if (pView->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
				return TRUE;
		}
	}

	return CView::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

void CTimeLineView::OnViewHide()
{
	m_wndTimeLine.HideSpans();
}

void CTimeLineView::OnViewUnhide()
{
	m_wndTimeLine.ShowAllSpans();
}
