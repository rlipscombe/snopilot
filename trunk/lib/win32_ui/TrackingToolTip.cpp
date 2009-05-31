#include "StdAfx.h"
#include "TrackingToolTip.h"
#include "win32/VersionInfo.h"

void CTrackingToolTip::Show(CWnd *pWnd, CPoint ptScreen, LPCTSTR lpszText)
{
    if (!Create(pWnd))
        return;

    /* Move the tooltip off the screen, so that it doesn't flicker when
     * it's resized with the new text.
     */
	if (!IsComCtl6())
	{
		int xOffScreen = -4000;
		int yOffScreen = -4000;
		m_pCtrl->SendMessage(TTM_TRACKPOSITION, 0, MAKELONG(xOffScreen, yOffScreen));
	}

    TOOLINFO ti;
    FillToolInfo(pWnd, &ti);
    m_pCtrl->SendMessage(TTM_TRACKACTIVATE, TRUE, (LPARAM)&ti);

    m_strText = lpszText;
    m_pCtrl->UpdateTipText(m_strText, pWnd);

    SetPos(pWnd, ptScreen);
}

void CTrackingToolTip::Hide()
{
    if (m_pCtrl)
    {
        m_pCtrl->DestroyWindow();
        delete m_pCtrl;
        m_pCtrl = NULL;
    }
}

BOOL CTrackingToolTip::Create(CWnd *pWnd)
{
    if (m_pCtrl)
        return TRUE;	// Already created.

    m_pCtrl = new CToolTipCtrl;
    if (m_pCtrl && m_pCtrl->Create(pWnd))
    {
        TOOLINFO ti;
        FillToolInfo(pWnd, &ti);

        // We have to use SendMessage, because CToolTipCtrl::AddTool doesn't pass all of the flags.
        m_pCtrl->SendMessage(TTM_ADDTOOL, 0, (LPARAM)&ti);
        m_pCtrl->SetDelayTime(TTDT_AUTOPOP, SHRT_MAX);   // stop the tooltip coming up automatically
        m_pCtrl->SetDelayTime(TTDT_INITIAL, 0);

		m_pCtrl->SetMaxTipWidth(400);

        return TRUE;	// Created successfully.
    }

    // Something failed.  Clean up and leave.
    delete m_pCtrl;
    return FALSE;
}

void CTrackingToolTip::SetPos(CWnd *pWnd, CPoint ptScreen)
{
    m_pCtrl->SendMessage(TTM_TRACKPOSITION, 0, MAKELONG(ptScreen.x, ptScreen.y));

	// For some reason, the view isn't redrawn properly when the mouse moves, so we have to give it a kick:
	if (!IsComCtl6())
		pWnd->UpdateWindow();
}

void CTrackingToolTip::FillToolInfo(CWnd *pWnd, TOOLINFO *ti)
{
    memset(ti, 0, sizeof(TOOLINFO));
    ti->cbSize = sizeof(TOOLINFO);
    ti->hwnd = pWnd->GetParent()->GetSafeHwnd();
    ti->uFlags = TTF_IDISHWND | TTF_ABSOLUTE | TTF_TRACK;
    ti->uId = reinterpret_cast<UINT_PTR>(pWnd->GetSafeHwnd());
}

