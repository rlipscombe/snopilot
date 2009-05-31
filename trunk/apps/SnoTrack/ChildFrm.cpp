// ChildFrm.cpp : implementation of the CChildFrame class
//
#include "stdafx.h"
#include "ChildFrm.h"
#include "IntroView.h"
#include "CartesianView.h"
#include "SidebarView.h"
#include "TimeLineView.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CChildFrame

IMPLEMENT_DYNCREATE(CChildFrame, CMDIChildWnd)

#define WM_REPLACE_VIEW (WM_USER + 101)

BEGIN_MESSAGE_MAP(CChildFrame, CMDIChildWnd)
	ON_WM_DESTROY()
	ON_MESSAGE(WM_REPLACE_VIEW, OnReplaceView)
	ON_WM_SIZE()
END_MESSAGE_MAP()


// CChildFrame construction/destruction

CChildFrame::CChildFrame()
{
	// TODO: add member initialization code here
}

CChildFrame::~CChildFrame()
{
}


BOOL CChildFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if (!CMDIChildWnd::PreCreateWindow(cs))
		return FALSE;

	return TRUE;
}


// CChildFrame diagnostics

#ifdef _DEBUG
void CChildFrame::AssertValid() const
{
	CMDIChildWnd::AssertValid();
}

void CChildFrame::Dump(CDumpContext& dc) const
{
	CMDIChildWnd::Dump(dc);
}

#endif //_DEBUG

// CChildFrame message handlers
const int INITIAL_TIMELINE_HEIGHT = 150;
const int INITIAL_SIDEBAR_WIDTH = 300;

BOOL CChildFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext)
{
	CRuntimeClass *pInitialViewClass;
	CSnoTrackDoc *pDoc = static_cast<CSnoTrackDoc *>(pContext->m_pCurrentDoc);
	ASSERT_KINDOF(CSnoTrackDoc, pDoc);
	
	// What's this?  But surely the document hasn't actually been loaded yet?
	// You'd be correct: normally (i.e. we're still in CDocTemplate::OpenDocumentFile), it wouldn't have
	// been, and we can always create the intro view.  We'll switch CCartesianView in once the 
	// data's loaded (via OnUpdate).
	//
	// However, if the user picks Window|New Window, we'll come back through here, and the document
	// will already have some data in it.
	// This time, OnUpdate won't get called, so we can't switch it in.
	//
	// Note that we have to be careful about the OnUpdate thing -- it's a bad idea to be deleting views
	// in it.
	if (pDoc->IsEmptyDataSet())
		pInitialViewClass = RUNTIME_CLASS(CIntroView);
	else
		pInitialViewClass = RUNTIME_CLASS(CCartesianView);

	/* The child frame area is split into 3 regions, like this:

      +---------+---+
	  |    1    | 2 |
	  |         |   |
	  +---------+---+
	  |      3      |
	  +-------------+

	 */

	// So, we need two splitters:
	if (!m_wndHorizSplitter.CreateStatic(this, 2, 1))
		return FALSE;

	if (!m_wndVertSplitter.CreateStatic(&m_wndHorizSplitter, 1, 2))
		return FALSE;

	CSize pane1Size(lpcs->cx - INITIAL_SIDEBAR_WIDTH, lpcs->cy - INITIAL_TIMELINE_HEIGHT);
	CSize pane2Size(INITIAL_SIDEBAR_WIDTH, lpcs->cy - INITIAL_TIMELINE_HEIGHT);
	CSize pane3Size(lpcs->cx, INITIAL_TIMELINE_HEIGHT);

	// Pane 1
	if (!m_wndVertSplitter.CreateView(0, 0, pInitialViewClass, pane1Size, pContext))
		return FALSE;

	// Pane 2
	if (!m_wndVertSplitter.CreateView(0, 1, RUNTIME_CLASS(CSidebarView), pane2Size, pContext))
		return FALSE;

	// Pane 3
	if (!m_wndHorizSplitter.CreateView(1, 0, RUNTIME_CLASS(CTimeLineView), pane3Size, pContext))
		return FALSE;

	// Tweak the sizes so that they're vaguely correct when initially shown.
	m_wndVertSplitter.SetColumnInfo(0, lpcs->cx - INITIAL_SIDEBAR_WIDTH, 0);
	m_wndVertSplitter.SetColumnInfo(1, INITIAL_SIDEBAR_WIDTH, 0);
	m_wndHorizSplitter.SetRowInfo(0, lpcs->cy - INITIAL_TIMELINE_HEIGHT, 0);
	m_wndHorizSplitter.SetRowInfo(1, INITIAL_TIMELINE_HEIGHT, 0);

	return TRUE;
}

void CChildFrame::ActivateFrame(int nCmdShow)
{
#if MAXIMIZE_MDI_CHILDREN
	if (GetMDIFrame()->MDIGetActive())
		CMDIChildWnd::ActivateFrame(nCmdShow);
	else
		CMDIChildWnd::ActivateFrame(SW_SHOWMAXIMIZED);
#else
	CMDIChildWnd::ActivateFrame(nCmdShow);
#endif
}

void CChildFrame::OnDestroy()
{
	CMDIChildWnd::OnDestroy();

	// TODO: Add your message handler code here
}

void CChildFrame::DeferReplaceView(CRuntimeClass *pNewViewClass)
{
	PostMessage(WM_REPLACE_VIEW, 0, (LPARAM)pNewViewClass);
}

LRESULT CChildFrame::OnReplaceView(WPARAM wParam, LPARAM lParam)
{
	CRuntimeClass *pNewViewClass = (CRuntimeClass *)lParam;

	ReplaceView(pNewViewClass);
	return 0;
}

BOOL CChildFrame::ReplaceView(CRuntimeClass *pNewViewClass)
{
	CWnd *pWnd = m_wndVertSplitter.GetPane(0, 0);
	ASSERT(pWnd);

	CRect rect;
	pWnd->GetWindowRect(rect);

	return m_wndVertSplitter.ReplaceView(0, 0, pNewViewClass, CSize(rect.Width(), rect.Height()));
}

CCartesianView *CChildFrame::GetCartesianView()
{
	CWnd *pWnd = m_wndVertSplitter.GetPane(0, 0);
	if (pWnd && pWnd->IsKindOf(RUNTIME_CLASS(CCartesianView)))
		return static_cast<CCartesianView *>(pWnd);

	return NULL;
}

void CChildFrame::OnUpdateFrameMenu(BOOL bActive, CWnd* pActiveWnd, HMENU hMenuAlt)
{
#if defined(ENABLE_DESIGN_MODE)
	CMenu *pMenu = CMenu::FromHandle(m_hMenuShared);
	UINT itemCount = pMenu->GetMenuItemCount();

	// Find out if our menu is already present:
	MENUITEMINFO mii;
	memset(&mii, 0, sizeof(MENUITEMINFO));
	mii.cbSize = sizeof(MENUITEMINFO);
	if (!pMenu->GetMenuItemInfo(ID_HOTSPOTS_DEFINEHOTSPOTS, &mii, FALSE))
	{
		CMenu menu;
		VERIFY(menu.LoadMenu(IDR_DESIGN_MODE));
		
		for (UINT i = 0; i < menu.GetMenuItemCount(); ++i)
		{
			CString str;
			menu.GetMenuString(i, str, MF_BYPOSITION);
			
			CMenu *pPopup = menu.GetSubMenu(i);
			pMenu->InsertMenu(itemCount - 2, MF_BYPOSITION | MF_POPUP, (UINT_PTR)pPopup->GetSafeHmenu(), str);
		}

		menu.Detach();
	}
#endif

	CMDIChildWnd::OnUpdateFrameMenu(bActive, pActiveWnd, hMenuAlt);
}

void CChildFrame::OnSize(UINT nType, int cx, int cy)
{
	CMDIChildWnd::OnSize(nType, cx, cy);
}
