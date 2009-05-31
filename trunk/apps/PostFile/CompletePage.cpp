// CompletePage.cpp : implementation file
//

#include "stdafx.h"
#include "CompletePage.h"
#include "PostFileSettings.h"
#include "win32/format_error.h"

// CCompletePage dialog
IMPLEMENT_DYNAMIC(CCompletePage, CWizardPage)

CCompletePage::CCompletePage(PostFileSettings *pSettings, PostFileResults *pResults)
	: CWizardPage(CCompletePage::IDD, IDS_COMPLETE_PAGE_CAPTION),
		m_pSettings(pSettings), m_pResults(pResults)
{
    m_psp.dwFlags |= PSP_HIDEHEADER;
}

CCompletePage::~CCompletePage()
{
}

void CCompletePage::DoDataExchange(CDataExchange* pDX)
{
	CWizardPage::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CCompletePage, CWizardPage)
END_MESSAGE_MAP()

// CCompletePage message handlers
BOOL CCompletePage::OnInitDialog()
{
	CWizardPage::OnInitDialog();

	InflictBoldFont(IDC_TITLE);

	return TRUE;
}

BOOL CCompletePage::OnSetActive()
{
	if (!CWizardPage::OnSetActive())
		return FALSE;

	EnableCancelButton(FALSE);

	if ((m_pResults->m_dwStatusCode / 100) == 2)
	{
		// Success
		CString title;
		title.LoadString(IDS_SUCCESSFUL_UPLOAD_TITLE);
		SetDlgItemText(IDC_TITLE, title);
		
		HICON hIcon = AfxGetApp()->LoadStandardIcon(MAKEINTRESOURCE(IDI_INFORMATION));
		CStatic *pStatic = static_cast<CStatic *>(GetDlgItem(IDC_COMPLETION_ICON));
		if (pStatic)
			pStatic->SetIcon(hIcon);

		CString message;
		message.Format(IDS_SUCCESSFUL_UPLOAD_MESSAGE,
			(LPCTSTR)m_pSettings->m_strLocalFilename);
		SetDlgItemText(IDC_MESSAGE, message);

		CString prompt;
		prompt.LoadString(IDS_SUCCESSFUL_UPLOAD_PROMPT);
		SetDlgItemText(IDC_PROMPT, prompt);

		SetWizardButtons(PSWIZB_BACK|PSWIZB_FINISH);
	}
	else
	{
		// Failure
		CString title;
		title.LoadString(IDS_FAILED_UPLOAD_TITLE);
		SetDlgItemText(IDC_TITLE, title);

		HICON hIcon = AfxGetApp()->LoadStandardIcon(MAKEINTRESOURCE(IDI_ERROR));

		CString message;
		if (m_pResults->m_dwStatusCode != -1)
		{
			// Then we attempted to upload the file.
			message.Format(IDS_FAILED_UPLOAD_MESSAGE,
				(LPCTSTR)m_pSettings->m_strLocalFilename,
				m_pResults->m_dwStatusCode, (LPCTSTR)m_pResults->m_strStatusText);
		}
		else
		{
			if (m_pResults->m_hResult == E_USER_CANCELLED)
			{
				CString title;
				title.LoadString(IDS_CANCELLED_UPLOAD_TITLE);//""
				SetDlgItemText(IDC_TITLE, title);

				message.Format(IDS_CANCELLED_UPLOAD_MESSAGE, (LPCTSTR)m_pSettings->m_strLocalFilename);
				
				hIcon = AfxGetApp()->LoadStandardIcon(MAKEINTRESOURCE(IDI_WARNING));
			}
			else
			{
				// Then we didn't get that far.
				message.Format(IDS_FAILED_UPLOAD_MESSAGE2,
					(LPCTSTR)m_pSettings->m_strLocalFilename,
					(LPCTSTR)FormatErrorMessage(m_pResults->m_hResult));
			}
		}

		CStatic *pStatic = static_cast<CStatic *>(GetDlgItem(IDC_COMPLETION_ICON));
		if (pStatic)
			pStatic->SetIcon(hIcon);

		SetDlgItemText(IDC_MESSAGE, message);

		CString prompt;
		prompt.LoadString(IDS_FAILED_UPLOAD_PROMPT);
		SetDlgItemText(IDC_PROMPT, prompt);

		SetWizardButtons(PSWIZB_BACK|PSWIZB_FINISH);
	}

	return TRUE;
}

LRESULT CCompletePage::OnWizardBack()
{
	return IDD_WELCOME_PAGE;
	// return CWizardPage::OnWizardBack();
}
