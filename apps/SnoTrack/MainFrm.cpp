// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "MainFrm.h"
#include <dde.h>
#include "resource.h"
#include "win32_ui/ToolBar32.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
    ON_WM_EXITMENULOOP()

	ON_UPDATE_COMMAND_UI(ID_INDICATOR_LON, OnUpdateLongitude)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_LAT, OnUpdateLatitude)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_SCALE, OnUpdateScale)

	// Workaround for a bug in MFC7.
	// See http://groups.google.com/groups?selm=yRPGhsEPDHA.2688%40cpmsftngxa06.phx.gbl
	ON_MESSAGE(WM_DDE_EXECUTE, OnDDEExecute)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_LON,
	ID_INDICATOR_LAT,
	ID_INDICATOR_SCALE,
};


// CMainFrame construction/destruction
CMainFrame::CMainFrame()
{
}

CMainFrame::~CMainFrame()
{
}

#define WINDOWPLACEMENT_FORMAT "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d"

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (!m_wndToolBar.CreateEx(this, TBSTYLE_TRANSPARENT | TBSTYLE_FLAT) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	Load32BitColorToolBar(&m_wndToolBar, IDB_MAINFRAME32);

	if (!m_wndReBar.Create(this) ||
		!m_wndReBar.AddBar(&m_wndToolBar))
	{
		TRACE("Failed to create rebar\n");
		return -1;      // fail to create
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE("Failed to create status bar\n");
		return -1;      // fail to create
	}
	// TODO: Remove this if you don't want tool tips
	m_wndToolBar.SetBarStyle(m_wndToolBar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY);

	CString s = AfxGetApp()->GetProfileString(_T("General"), _T("Window Placement"));
	if (!s.IsEmpty())
	{
		TRACE("s = %s\n", (LPCTSTR)s);

		WINDOWPLACEMENT wp;
		wp.length = sizeof(WINDOWPLACEMENT);

		int n = sscanf(s, WINDOWPLACEMENT_FORMAT, &wp.flags, &wp.showCmd,
			&wp.ptMinPosition.x, &wp.ptMinPosition.y, &wp.ptMaxPosition.x, &wp.ptMaxPosition.y,
			&wp.rcNormalPosition.left, &wp.rcNormalPosition.top, &wp.rcNormalPosition.right, &wp.rcNormalPosition.bottom);

		if (n == 10)
			VERIFY(SetWindowPlacement(&wp));
	}

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if (!CMDIFrameWnd::PreCreateWindow(cs))
		return FALSE;

	return TRUE;
}


// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CMDIFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CMDIFrameWnd::Dump(dc);
}

#endif //_DEBUG


// CMainFrame message handlers
void CMainFrame::OnDestroy()
{
	WINDOWPLACEMENT wp;
	if (GetWindowPlacement(&wp))
	{
		CString s;
		s.Format(WINDOWPLACEMENT_FORMAT,
					wp.flags, wp.showCmd,
					wp.ptMinPosition.x, wp.ptMinPosition.y,
					wp.ptMaxPosition.x, wp.ptMaxPosition.y,
					wp.rcNormalPosition.left, wp.rcNormalPosition.top, wp.rcNormalPosition.right, wp.rcNormalPosition.bottom);

		AfxGetApp()->WriteProfileString(_T("General"), _T("Window Placement"), s);
	}

	delete m_wndToolBar.GetToolBarCtrl().GetImageList();
	delete m_wndToolBar.GetToolBarCtrl().GetHotImageList();
	delete m_wndToolBar.GetToolBarCtrl().GetDisabledImageList();

	CMDIFrameWnd::OnDestroy();
}

void CMainFrame::OnExitMenuLoop(BOOL bIsTrackPopupMenu)
{
	SendMessageToDescendants(WM_EXITMENULOOP, bIsTrackPopupMenu);
}

LRESULT CMainFrame::OnDDEExecute(WPARAM wParam, LPARAM lParam)
{
	// unpack the DDE message
	UINT_PTR unused;
	HGLOBAL hData;
	//IA64: Assume DDE LPARAMs are still 32-bit
	VERIFY(UnpackDDElParam(WM_DDE_EXECUTE, lParam, &unused, (UINT_PTR*)&hData));

	// get the command string
	TCHAR szCommand[_MAX_PATH * 2];
	LPCTSTR lpsz = (LPCTSTR)GlobalLock(hData);
	int commandLength = lstrlen(lpsz);
	if (commandLength >= COUNTOF(szCommand))
	{
		// The command would be truncated. This could be a security problem
		TRACE("Warning: Command was ignored because it was too long.\n");
		return 0;
	}

	lstrcpyn(szCommand, lpsz, COUNTOF(szCommand));
	GlobalUnlock(hData);

	// acknowledge now - before attempting to execute
	::PostMessage((HWND)wParam, WM_DDE_ACK, (WPARAM)m_hWnd,
		//IA64: Assume DDE LPARAMs are still 32-bit
		ReuseDDElParam(lParam, WM_DDE_EXECUTE, WM_DDE_ACK,
		(UINT)0x8000, (UINT_PTR)hData));

	// don't execute the command when the window is disabled
	if (!IsWindowEnabled())
	{
		ATLTRACE(traceAppMsg, 0, _T("Warning: DDE command '%s' ignored because window is disabled.\n"),
			szCommand);
		return 0;
	}

	// execute the command
	if (!AfxGetApp()->OnDDECommand(szCommand))
		ATLTRACE(traceAppMsg, 0, _T("Error: failed to execute DDE command '%s'.\n"), szCommand);

	return 0L;
}

void CMainFrame::OnUpdateLongitude(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(FALSE);
}

void CMainFrame::OnUpdateLatitude(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(FALSE);
}

void CMainFrame::OnUpdateScale(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(FALSE);
}
