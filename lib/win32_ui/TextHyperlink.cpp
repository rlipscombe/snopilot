// TextHyperlink.cpp : implementation file
//

#include "stdafx.h"
#include "TextHyperlink.h"


// CTextHyperlink

IMPLEMENT_DYNAMIC(CTextHyperlink, CStatic)

CTextHyperlink::CTextHyperlink()
	: m_hCursor(NULL)
{
}

CTextHyperlink::~CTextHyperlink()
{
}


BEGIN_MESSAGE_MAP(CTextHyperlink, CStatic)
	ON_WM_CTLCOLOR_REFLECT()
	ON_CONTROL_REFLECT(STN_CLICKED, OnClicked)
	ON_WM_SETCURSOR()
END_MESSAGE_MAP()

// CTextHyperlink message handlers

HBRUSH CTextHyperlink::CtlColor(CDC *pDC, UINT nCtlColor)
{
	// Turn on the notify flag, so we get the clicks:
	DWORD dwStyle = GetStyle();
	if (!(dwStyle & SS_NOTIFY))
		ModifyStyle(0, SS_NOTIFY, 0);

	HBRUSH hbr = NULL;
	if ((dwStyle & SS_TYPEMASK) <= SS_RIGHT)
	{
		// It's a normal text control.
		if (!m_font.GetSafeHandle())
		{
			// First time: set some stuff up.
			CFont *pCurrentFont = GetFont();
			ASSERT(pCurrentFont);

			LOGFONT lf;
			pCurrentFont->GetLogFont(&lf);

			lf.lfUnderline = TRUE;
			VERIFY(m_font.CreateFontIndirect(&lf));
		}

		pDC->SelectObject(&m_font);
		pDC->SetTextColor(RGB(0,0,255));
		pDC->SetBkMode(TRANSPARENT);

		hbr = (HBRUSH)::GetStockObject(HOLLOW_BRUSH);
	}

	return hbr;
}

extern CString FormatShellExecuteErrorMessage(INT_PTR nResult, LPCTSTR lpszPathName);

void CTextHyperlink::OnClicked()
{
	CString link;
	GetWindowText(link);

	HCURSOR hCurAppStarting = AfxGetApp()->LoadStandardCursor(MAKEINTRESOURCE(IDC_APPSTARTING));
	HCURSOR hCurRestore = ::SetCursor(hCurAppStarting);

	INT_PTR result = (INT_PTR)ShellExecute(NULL, _T("open"), link, NULL, NULL, SW_SHOWNORMAL);

	if (::GetCursor() == hCurAppStarting)
		::SetCursor(hCurRestore);

	if (result <= 32)
	{
		// Then it failed: prompt the user.
		CString error = FormatShellExecuteErrorMessage(result, link);
		AfxMessageBox(error, MB_ICONEXCLAMATION);
		return;
	}
}

BOOL CTextHyperlink::OnSetCursor(CWnd *pWnd, UINT nHitTest, UINT message)
{
	if (!m_hCursor)
		m_hCursor = AfxGetApp()->LoadStandardCursor(MAKEINTRESOURCE(IDC_HAND));

	if (m_hCursor)
	{
		::SetCursor(m_hCursor);
		return TRUE;
	}

	return FALSE;
}
