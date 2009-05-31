/* CompletePage.cpp
 *
 * Copyright (C) 2003 Roger Lipscombe
 *     http://www.differentpla.net/~roger/
 */

#include "StdAfx.h"
#include "ImportSerialCompletePage.h"
#include "ImportSerialSettings.h"
#include "win32_ui/bold_font.h"

IMPLEMENT_DYNAMIC(CImportSerialCompletePage, CWizardPage)

/////////////////////////////////////////////////////////////////////////////
// CImportSerialCompletePage property page

CImportSerialCompletePage::CImportSerialCompletePage(const ImportSerialResults *pResults)
    : CWizardPage(CImportSerialCompletePage::IDD, IDS_IMPORT_SERIAL_WIZARD_CAPTION),
		m_pResults(pResults)
{
    //{{AFX_DATA_INIT(CImportSerialCompletePage)
    //}}AFX_DATA_INIT

    m_psp.dwFlags |= PSP_HIDEHEADER;
}

CImportSerialCompletePage::~CImportSerialCompletePage()
{
}

void CImportSerialCompletePage::DoDataExchange(CDataExchange* pDX)
{
    CWizardPage::DoDataExchange(pDX);

    //{{AFX_DATA_MAP(CImportSerialCompletePage)
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CImportSerialCompletePage, CWizardPage)
    //{{AFX_MSG_MAP(CImportSerialCompletePage)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CImportSerialCompletePage::OnInitDialog() 
{
	CWizardPage::OnInitDialog();
	InflictBoldFont(IDC_TITLE);

	return TRUE;
}

BOOL CImportSerialCompletePage::OnSetActive() 
{
	if (!CWizardPage::OnSetActive())
		return FALSE;

	STATUS taskStatus = m_pResults->GetTaskStatus();
	if (taskStatus == E_USER_CANCELLED)
	{
		CString title;
		title.LoadString(IDS_IMPORT_SERIAL_CANCELLED_TITLE);
		SetDlgItemText(IDC_TITLE, title);

		CString message;
		message.LoadString(IDS_IMPORT_SERIAL_CANCELLED);
		SetDlgItemText(IDC_COMPLETION_MESSAGE, message);
		SetWizardButtons(PSWIZB_BACK|PSWIZB_FINISH);

		HICON hIcon = AfxGetApp()->LoadStandardIcon(MAKEINTRESOURCE(IDI_WARNING));
		
		CStatic *pStatic = static_cast<CStatic *>(GetDlgItem(IDC_COMPLETION_ICON));
		pStatic->SetIcon(hIcon);
	}
	else if (FAILED(taskStatus))
	{
		CString title;
		title.LoadString(IDS_IMPORT_SERIAL_FAILED_TITLE);
		SetDlgItemText(IDC_TITLE, title);

		CString message;
		message.FormatMessage(IDS_IMPORT_SERIAL_FAILED, taskStatus);
		SetDlgItemText(IDC_COMPLETION_MESSAGE, message);
		SetWizardButtons(PSWIZB_BACK|PSWIZB_FINISH);

		HICON hIcon = AfxGetApp()->LoadStandardIcon(MAKEINTRESOURCE(IDI_ERROR));

		CStatic *pStatic = static_cast<CStatic *>(GetDlgItem(IDC_COMPLETION_ICON));
		pStatic->SetIcon(hIcon);
	}
	else
	{
		CString title;
		title.LoadString(IDS_IMPORT_SERIAL_SUCCEEDED_TITLE);
		SetDlgItemText(IDC_TITLE, title);

		CString message;
		message.FormatMessage(IDS_IMPORT_SERIAL_SUCCEEDED, m_pResults->GetPositionCount(), m_pResults->GetWaypointCount());
		SetDlgItemText(IDC_COMPLETION_MESSAGE, message);
		SetWizardButtons(PSWIZB_FINISH);

		HICON hIcon = AfxGetApp()->LoadStandardIcon(MAKEINTRESOURCE(IDI_INFORMATION));
		
		CStatic *pStatic = static_cast<CStatic *>(GetDlgItem(IDC_COMPLETION_ICON));
		pStatic->SetIcon(hIcon);
	}

	return TRUE;
}

LRESULT CImportSerialCompletePage::OnWizardBack() 
{
	// By default, this would return us to the progress page.  We probably don't want that,
	// because there's nothing interesting there, and the user is probably pressing Back because they
	// want to configure something.
	return IDD_IMPORT_SERIAL_DEVICE_PAGE;

//	return CWizardPage::OnWizardBack();
}
