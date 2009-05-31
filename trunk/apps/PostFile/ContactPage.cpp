// ContactPage.cpp : implementation file
//

#include "stdafx.h"
#include "ContactPage.h"
#include "PostFileSettings.h"

// CContactPage dialog

IMPLEMENT_DYNAMIC(CContactPage, CWizardPage)

CContactPage::CContactPage(PostFileSettings *pSettings)
	: CWizardPage(CContactPage::IDD, IDS_CONTACT_PAGE_CAPTION, IDS_CONTACT_PAGE_TITLE, IDS_CONTACT_PAGE_SUBTITLE), m_pSettings(pSettings)
{
}

CContactPage::~CContactPage()
{
}

void CContactPage::DoDataExchange(CDataExchange* pDX)
{
	CWizardPage::DoDataExchange(pDX);
	
	DDX_Control(pDX, IDC_FORUMS_URL, m_forumLink);
	DDX_Text(pDX, IDC_EMAIL_ADDRESS, m_pSettings->m_strEmailAddress);
	DDX_Text(pDX, IDC_FORUM_USERNAME, m_pSettings->m_strForumUserName);
	DDX_Check(pDX, IDC_ANONYMOUS, m_pSettings->m_bAnonymous);
}

BEGIN_MESSAGE_MAP(CContactPage, CWizardPage)
	ON_BN_CLICKED(IDC_ANONYMOUS, OnBnClickedAnonymous)
END_MESSAGE_MAP()

// CContactPage message handlers
BOOL CContactPage::OnSetActive()
{
	if (!CWizardPage::OnSetActive())
		return FALSE;

	SetWizardButtons(PSWIZB_BACK|PSWIZB_NEXT);
	EnableCancelButton(TRUE);

	EnableControls();
	return TRUE;
}

void CContactPage::OnBnClickedAnonymous()
{
	EnableControls();
}

void CContactPage::EnableControls()
{
	BOOL bEnable;
	if (IsDlgButtonChecked(IDC_ANONYMOUS))
		bEnable = FALSE;
	else
		bEnable = TRUE;

	GetDlgItem(IDC_EMAIL_ADDRESS)->EnableWindow(bEnable);
	GetDlgItem(IDC_EMAIL_ADDRESS_LABEL)->EnableWindow(bEnable);
	GetDlgItem(IDC_EMAIL_ADDRESS_DESCRIPTION)->EnableWindow(bEnable);

	GetDlgItem(IDC_FORUM_USERNAME)->EnableWindow(bEnable);
	GetDlgItem(IDC_FORUM_USERNAME_LABEL)->EnableWindow(bEnable);
	GetDlgItem(IDC_FORUM_USERNAME_DESCRIPTION)->EnableWindow(bEnable);
}

LRESULT CContactPage::OnWizardNext()
{
	// If you're not anonymous, you need to have entered one of the email address/forum user name fields.
	if (!IsDlgButtonChecked(IDC_ANONYMOUS))
	{
		CString strEmailAddress;
		CString strForumUserName;

		GetDlgItemText(IDC_EMAIL_ADDRESS, strEmailAddress);
		GetDlgItemText(IDC_FORUM_USERNAME, strForumUserName);

		if (strEmailAddress.IsEmpty() && strForumUserName.IsEmpty())
		{
			AfxMessageBox(IDS_EMAIL_OR_FORUM_OR_ANON);
			return -1;
		}
	}

	return CWizardPage::OnWizardNext();
}
