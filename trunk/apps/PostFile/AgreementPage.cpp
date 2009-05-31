// AgreementPage.cpp : implementation file
//

#include "stdafx.h"
#include "AgreementPage.h"
#include "PostFileSettings.h"

// CAgreementPage dialog
IMPLEMENT_DYNAMIC(CAgreementPage, CWizardPage)

CAgreementPage::CAgreementPage(PostFileSettings *pSettings)
	: CWizardPage(CAgreementPage::IDD, IDS_AGREEMENT_PAGE_CAPTION, IDS_AGREEMENT_PAGE_TITLE, IDS_AGREEMENT_PAGE_SUBTITLE),
		m_pSettings(pSettings)
{
}

CAgreementPage::~CAgreementPage()
{
}

void CAgreementPage::DoDataExchange(CDataExchange* pDX)
{
	CWizardPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_AGREEMENT, m_agreementCtrl);
}


BEGIN_MESSAGE_MAP(CAgreementPage, CWizardPage)
	ON_BN_CLICKED(IDC_DO_NOT_AGREE, OnBnClickedDoNotAgree)
	ON_BN_CLICKED(IDC_AGREE, OnBnClickedAgree)
END_MESSAGE_MAP()


// CAgreementPage message handlers

BOOL CAgreementPage::OnInitDialog()
{
	CWizardPage::OnInitDialog();

	CheckRadioButton(IDC_DO_NOT_AGREE, IDC_AGREE, IDC_DO_NOT_AGREE);

	// Load the RTF text:
	HINSTANCE hinst = AfxFindResourceHandle(MAKEINTRESOURCE(IDR_AGREEMENT), _T("RTF"));
	HRSRC hrsrc = FindResource(hinst, MAKEINTRESOURCE(IDR_AGREEMENT), _T("RTF"));
	unsigned cb = SizeofResource(hinst, hrsrc);
	HGLOBAL hglb = LoadResource(hinst, hrsrc);
	char *pData = (char *)LockResource(hglb);

	CString agreementText(pData, cb);
	m_agreementCtrl.SetWindowText(agreementText);

	UnlockResource(hglb);
	FreeResource(hglb);

	return TRUE;
}

void CAgreementPage::EnableButtons()
{
	if (GetCheckedRadioButton(IDC_DO_NOT_AGREE, IDC_AGREE) == IDC_DO_NOT_AGREE)
        SetWizardButtons(PSWIZB_BACK);
	else
		SetWizardButtons(PSWIZB_BACK|PSWIZB_NEXT);

	EnableCancelButton(TRUE);
}

void CAgreementPage::OnBnClickedDoNotAgree()
{
	m_pSettings->m_bAgree = (GetCheckedRadioButton(IDC_DO_NOT_AGREE, IDC_AGREE) == IDC_AGREE);
	EnableButtons();
}

void CAgreementPage::OnBnClickedAgree()
{
	m_pSettings->m_bAgree = (GetCheckedRadioButton(IDC_DO_NOT_AGREE, IDC_AGREE) == IDC_AGREE);
	EnableButtons();
}

BOOL CAgreementPage::OnSetActive()
{
	if (!CWizardPage::OnSetActive())
		return FALSE;

	UINT nRadio = m_pSettings->m_bAgree ? IDC_AGREE : IDC_DO_NOT_AGREE;
	CheckRadioButton(IDC_DO_NOT_AGREE, IDC_AGREE, nRadio);
	EnableButtons();

	return TRUE;
}
