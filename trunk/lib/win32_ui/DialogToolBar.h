/* DialogToolBar.h
 */

#pragma once

class CDialogToolBar : public CToolBar
{
    DECLARE_DYNAMIC(CDialogToolBar)

    //{{AFX_VIRTUAL(CDialogToolBar)
    //}}AFX_VIRTUAL

    //{{AFX_MSG(CDialogToolBar)
    afx_msg LRESULT OnIdleUpdateCmdUI(WPARAM wParam, LPARAM lParam);
    afx_msg void OnInitialUpdate();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};
