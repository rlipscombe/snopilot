/* WelcomePage.cpp
 *
 * Copyright (C) 2003 Roger Lipscombe
 *     http://www.differentpla.net/~roger/
 */

#include "stdafx.h"
#include "ImportSerialWelcomePage.h"
#include "win32_ui/bold_font.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CImportSerialWelcomePage, CWizardPage)


/////////////////////////////////////////////////////////////////////////////
// CImportSerialWelcomePage property page

CImportSerialWelcomePage::CImportSerialWelcomePage()
    : CWizardPage(CImportSerialWelcomePage::IDD, IDS_IMPORT_SERIAL_WIZARD_CAPTION)
{
    //{{AFX_DATA_INIT(CImportSerialWelcomePage)
    //}}AFX_DATA_INIT

    m_psp.dwFlags |= PSP_HIDEHEADER;
}

CImportSerialWelcomePage::~CImportSerialWelcomePage()
{
}

void CImportSerialWelcomePage::DoDataExchange(CDataExchange* pDX)
{
    CWizardPage::DoDataExchange(pDX);

    //{{AFX_DATA_MAP(CImportSerialWelcomePage)
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CImportSerialWelcomePage, CWizardPage)
    //{{AFX_MSG_MAP(CImportSerialWelcomePage)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CImportSerialWelcomePage::OnInitDialog() 
{
    CWizardPage::OnInitDialog();

	InflictBoldFont(IDC_TITLE);

    return TRUE;
}

BOOL CImportSerialWelcomePage::OnSetActive() 
{
    if (!CWizardPage::OnSetActive())
		return FALSE;

    SetWizardButtons(PSWIZB_NEXT);

    return TRUE;
}
