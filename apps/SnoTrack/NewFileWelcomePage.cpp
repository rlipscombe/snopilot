// NewFileWelcomePage.cpp : implementation file
//

#include "stdafx.h"
#include "NewFileWelcomePage.h"
#include "win32_ui/bold_font.h"

// CNewFileWelcomePage dialog

IMPLEMENT_DYNAMIC(CNewFileWelcomePage, CWizardPage)

CNewFileWelcomePage::CNewFileWelcomePage()
	: CWizardPage(CNewFileWelcomePage::IDD, IDS_NEW_FILE_WELCOME_CAPTION)
{
    m_psp.dwFlags |= PSP_HIDEHEADER;
}

CNewFileWelcomePage::~CNewFileWelcomePage()
{
}

void CNewFileWelcomePage::DoDataExchange(CDataExchange* pDX)
{
	CWizardPage::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CNewFileWelcomePage, CWizardPage)
END_MESSAGE_MAP()

// CNewFileWelcomePage message handlers
BOOL CNewFileWelcomePage::OnInitDialog()
{
	CWizardPage::OnInitDialog();

	InflictBoldFont(IDC_TITLE);

	return TRUE;
}

BOOL CNewFileWelcomePage::OnSetActive()
{
    if (!CWizardPage::OnSetActive())
		return FALSE;

    SetWizardButtons(PSWIZB_NEXT);

    return TRUE;
}
