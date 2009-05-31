// NewFileCompletePage.cpp : implementation file
//

#include "stdafx.h"
#include "NewFileCompletePage.h"
#include "NewFileSettings.h"
#include "win32_ui/bold_font.h"

// CNewFileCompletePage dialog

IMPLEMENT_DYNAMIC(CNewFileCompletePage, CWizardPage)

CNewFileCompletePage::CNewFileCompletePage(const NewFileSettings *pSettings)
	: CWizardPage(CNewFileCompletePage::IDD, IDS_NEW_FILE_COMPLETE_CAPTION),
		m_pSettings(pSettings)
{
    m_psp.dwFlags |= PSP_HIDEHEADER;
}

CNewFileCompletePage::~CNewFileCompletePage()
{
}

void CNewFileCompletePage::DoDataExchange(CDataExchange* pDX)
{
	CWizardPage::DoDataExchange(pDX);
}

#define WM_DEFERRED_INIT (WM_APP+22)

BEGIN_MESSAGE_MAP(CNewFileCompletePage, CWizardPage)
	ON_MESSAGE(WM_DEFERRED_INIT, OnDeferredInit)
END_MESSAGE_MAP()


// CNewFileCompletePage message handlers
BOOL CNewFileCompletePage::OnInitDialog()
{
	CWizardPage::OnInitDialog();

	InflictBoldFont(IDC_TITLE);

	return TRUE;
}

BOOL CNewFileCompletePage::OnSetActive()
{
	if (!CWizardPage::OnSetActive())
		return FALSE;

	if (m_pSettings->IsCustomResort())
	{
		SetDlgItemText(IDC_RESORT, m_pSettings->GetCustomResortName());
	}
	else
	{
		SetDlgItemText(IDC_RESORT, m_pSettings->GetResortPath());
	}

	SetDlgItemText(IDC_COMMENTS, m_pSettings->GetComments());

	SetWizardButtons(PSWIZB_BACK|PSWIZB_FINISH);
	PostMessage(WM_DEFERRED_INIT);

	return TRUE;
}

LRESULT CNewFileCompletePage::OnDeferredInit(WPARAM wParam, LPARAM lParam)
{
	CWnd *pFinishButton = static_cast<CPropertySheet *>(GetParent())->GetDlgItem(0x3025);
	if (pFinishButton)
		pFinishButton->SetFocus();

	return 0;
}
