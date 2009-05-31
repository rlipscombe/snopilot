// ImportFileProgressDialog.cpp : implementation file
//

#include "stdafx.h"
#include "ImportFileProgressDialog.h"


// CImportFileProgressDialog dialog

IMPLEMENT_DYNAMIC(CImportFileProgressDialog, CDialog)

CImportFileProgressDialog::CImportFileProgressDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CImportFileProgressDialog::IDD, pParent)
{
	m_bCancel = FALSE;
	m_bParentDisabled = FALSE;
}

CImportFileProgressDialog::~CImportFileProgressDialog()
{
	if (m_hWnd)
		DestroyWindow();
}

BOOL CImportFileProgressDialog::DestroyWindow()
{
    ReEnableParent();
    return CDialog::DestroyWindow();
}

void CImportFileProgressDialog::ReEnableParent()
{
	if (m_bParentDisabled && (m_pParentWnd != NULL))
		m_pParentWnd->EnableWindow(TRUE);

	m_bParentDisabled = FALSE;
}

BOOL CImportFileProgressDialog::Create(CWnd *pParent)
{
	// Get the true parent of the dialog
	m_pParentWnd = CWnd::GetSafeOwner(pParent);

	// m_bParentDisabled is used to re-enable the parent window
	// when the dialog is destroyed. So we don't want to set
	// it to TRUE unless the parent was already enabled.

	if (m_pParentWnd && m_pParentWnd->IsWindowEnabled())
	{
		m_pParentWnd->EnableWindow(FALSE);
		m_bParentDisabled = TRUE;
	}

	if (!CDialog::Create(CImportFileProgressDialog::IDD, pParent))
	{
		ReEnableParent();
		return FALSE;
	}

	return TRUE;
}

void CImportFileProgressDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROGRESS, m_progressCtrl);
}


BEGIN_MESSAGE_MAP(CImportFileProgressDialog, CDialog)
END_MESSAGE_MAP()

void CImportFileProgressDialog::PumpMessages()
{
	// Must call Create() before using the dialog
	ASSERT(m_hWnd!=NULL);

	MSG msg;
	// Handle dialog messages
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		if (!IsDialogMessage(&msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);  
		}
	}
}

STATUS CImportFileProgressDialog::OnProgress(int num, int denom)
{
	m_progressCtrl.SetRange32(0, denom);
	m_progressCtrl.SetPos(num);

	PumpMessages();

	return S_OK;
}

STATUS CImportFileProgressDialog::OnComplete()
{
	int lower, upper;
	m_progressCtrl.GetRange(lower, upper);
	m_progressCtrl.SetPos(upper);

	PumpMessages();

	return S_OK;
}

// CImportFileProgressDialog message handlers
