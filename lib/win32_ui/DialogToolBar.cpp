/* DialogToolBar.cpp
 */

#include "StdAfx.h"
#include "DialogToolBar.h"
#include <afxpriv.h>	// for WM_IDLEUPDATECMDUI

IMPLEMENT_DYNAMIC(CDialogToolBar, CToolBar)

BEGIN_MESSAGE_MAP(CDialogToolBar, CToolBar)
    //{{AFX_MSG_MAP(CDialogToolBar)
    ON_MESSAGE_VOID(WM_INITIALUPDATE, OnInitialUpdate)
    ON_MESSAGE(WM_IDLEUPDATECMDUI, OnIdleUpdateCmdUI)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CDialogToolBar::OnInitialUpdate()
{
    // update the indicators before becoming visible
    OnIdleUpdateCmdUI(TRUE, 0L);
}

LRESULT CDialogToolBar::OnIdleUpdateCmdUI(WPARAM wParam, LPARAM lParam)
{
	if (IsWindowVisible())
	{
		CFrameWnd* pParent = (CFrameWnd*)GetParent();
		if (pParent)
			OnUpdateCmdUI(pParent, (BOOL)wParam);
	}

	return 0L;
}
