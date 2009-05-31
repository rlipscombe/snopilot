/* WelcomePage.cpp
 *
 * Copyright (C) 2003 Roger Lipscombe
 *     http://www.differentpla.net/~roger/
 */

#include "stdafx.h"
#include "WelcomePage.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CWelcomePage, CWizardPage)

/////////////////////////////////////////////////////////////////////////////
// CWelcomePage property page

CWelcomePage::CWelcomePage()
    : CWizardPage(CWelcomePage::IDD, IDS_WELCOME_PAGE_CAPTION)
{
    //{{AFX_DATA_INIT(CWelcomePage)
    //}}AFX_DATA_INIT

    m_psp.dwFlags |= PSP_HIDEHEADER;
}

CWelcomePage::~CWelcomePage()
{
}

void CWelcomePage::DoDataExchange(CDataExchange* pDX)
{
    CWizardPage::DoDataExchange(pDX);

    //{{AFX_DATA_MAP(CWelcomePage)
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CWelcomePage, CWizardPage)
    //{{AFX_MSG_MAP(CWelcomePage)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CWelcomePage::OnInitDialog() 
{
    CWizardPage::OnInitDialog();

	InflictBoldFont(IDC_TITLE);

    return TRUE;
}

BOOL CWelcomePage::OnSetActive() 
{
    if (!CWizardPage::OnSetActive())
		return FALSE;

    SetWizardButtons(PSWIZB_NEXT);
	EnableCancelButton(TRUE);

    return TRUE;
}
