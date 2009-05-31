// NewFileResortPage.cpp : implementation file
//

#include "stdafx.h"
#include "NewFileResortPage.h"
#include "NewFileSettings.h"

// CNewFileResortPage dialog

IMPLEMENT_DYNAMIC(CNewFileResortPage, CWizardPage)

CNewFileResortPage::CNewFileResortPage(NewFileSettings *pSettings)
	: CWizardPage(CNewFileResortPage::IDD, IDS_NEW_FILE_RESORT_CAPTION, IDS_NEW_FILE_RESORT_TITLE, IDS_NEW_FILE_RESORT_SUBTITLE),
		m_pSettings(pSettings)
{
}

CNewFileResortPage::~CNewFileResortPage()
{
}

void CNewFileResortPage::DoDataExchange(CDataExchange* pDX)
{
	CWizardPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMMENTS, m_commentsEdit);
}

BEGIN_MESSAGE_MAP(CNewFileResortPage, CWizardPage)
	ON_BN_CLICKED(IDC_CUSTOM_RESORT, OnBnClickedCustomResort)
	ON_BN_CLICKED(IDC_PREDEFINED_RESORT, OnBnClickedPredefinedResort)
	ON_BN_CLICKED(IDC_BROWSE_RESORT_FILE, OnBnClickedBrowseResortFile)
	ON_EN_CHANGE(IDC_RESORT_FILE, OnEnChangeResortFile)
	ON_EN_CHANGE(IDC_RESORT_NAME, OnEnChangeResortName)
END_MESSAGE_MAP()

// CNewFileResortPage message handlers
BOOL CNewFileResortPage::OnInitDialog()
{
	CWizardPage::OnInitDialog();

	return TRUE;
}

BOOL CNewFileResortPage::OnSetActive()
{
	if (!CWizardPage::OnSetActive())
		return FALSE;

	SetWizardButtons(PSWIZB_BACK|PSWIZB_NEXT);

	// If the user last chose a custom resort, then choose that option:
	if (m_pSettings->IsCustomResort())
	{
		CheckRadioButton(IDC_PREDEFINED_RESORT, IDC_CUSTOM_RESORT, IDC_CUSTOM_RESORT);
		OnBnClickedCustomResort();
	}
	else
	{
		CheckRadioButton(IDC_PREDEFINED_RESORT, IDC_CUSTOM_RESORT, IDC_PREDEFINED_RESORT);
		SetDlgItemText(IDC_RESORT_FILE, m_pSettings->GetResortPath());
		OnBnClickedPredefinedResort();
	}

	EnableControls();

	return TRUE;
}

LRESULT CNewFileResortPage::OnWizardNext()
{
	int nCheck = GetCheckedRadioButton(IDC_PREDEFINED_RESORT, IDC_CUSTOM_RESORT);
	if (nCheck == IDC_CUSTOM_RESORT)
	{
		// If it's a custom resort, then it must be filled in.
		m_pSettings->SetCustomResort(true);

		CString str;
		GetDlgItemText(IDC_RESORT_NAME, str);
		if (!str.IsEmpty())
			m_pSettings->SetCustomResortName(str);
		else
			return -1;
	}
	else
	{
		// If it's a predefined resort, then _that_ must be filled in.
		m_pSettings->SetCustomResort(false);
		
		// We'll check to see if it's kosher later.
		CString str;
		GetDlgItemText(IDC_RESORT_FILE, str);
		if (!str.IsEmpty())
			m_pSettings->SetResortPath(str);
		else
			return -1;
	}

	// Save the user's comments:
	CString strComments;
	m_commentsEdit.GetWindowText(strComments);
	if (strComments.GetLength() > SHRT_MAX)
	{
		CString message;
		message.Format(IDS_COMMENTS_SIZE, SHRT_MAX);
		AfxMessageBox(message);
		return -1;
	}
	else
		m_pSettings->SetComments(strComments);

	return CWizardPage::OnWizardNext();
}

void CNewFileResortPage::OnBnClickedCustomResort()
{
	// The custom resort box is selected, so put the cursor in the relevant box.
	CEdit *pEdit = static_cast<CEdit *>(GetDlgItem(IDC_RESORT_NAME));
	pEdit->EnableWindow(TRUE);
	pEdit->SetFocus();
	pEdit->SetSel(0, -1);

	EnableControls();
}

void CNewFileResortPage::OnBnClickedPredefinedResort()
{
	// The predefined resort box is selected, so disable the resort name box.
	CEdit *pEdit = static_cast<CEdit *>(GetDlgItem(IDC_RESORT_NAME));
	pEdit->EnableWindow(FALSE);

	// Put the cursor in the filename box.
	GetDlgItem(IDC_RESORT_FILE)->SetFocus();

	EnableControls();
}

void CNewFileResortPage::EnableControls()
{
	int nCheck = GetCheckedRadioButton(IDC_PREDEFINED_RESORT, IDC_CUSTOM_RESORT);
	if (nCheck == IDC_CUSTOM_RESORT)
	{
		// If it's a custom resort, then it must be filled in.
		CString str;
		GetDlgItemText(IDC_RESORT_NAME, str);
		if (!str.IsEmpty())
			SetWizardButtons(PSWIZB_BACK|PSWIZB_NEXT);
		else
			SetWizardButtons(PSWIZB_BACK);
	}
	else
	{
		// If it's a predefined resort, then _that_ must be filled in.
		// We'll check to see if it's kosher later.
		CString str;
		GetDlgItemText(IDC_RESORT_FILE, str);
		if (!str.IsEmpty())
			SetWizardButtons(PSWIZB_BACK|PSWIZB_NEXT);
		else
			SetWizardButtons(PSWIZB_BACK);
	}
}

void CNewFileResortPage::OnBnClickedBrowseResortFile()
{
	CString strResortFile;
	GetDlgItemText(IDC_RESORT_FILE, strResortFile);

	CFileDialog dlg(TRUE, _T(".resort"), strResortFile, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, _T("Resort Files (*.resort)|*.resort||"), this);
	INT_PTR nResult = dlg.DoModal();
	if (nResult == IDOK)
	{
		SetDlgItemText(IDC_RESORT_FILE, dlg.GetPathName());
		CheckRadioButton(IDC_PREDEFINED_RESORT, IDC_CUSTOM_RESORT, IDC_PREDEFINED_RESORT);
		OnBnClickedPredefinedResort();
	}
}

void CNewFileResortPage::OnEnChangeResortFile()
{
	EnableControls();
}

void CNewFileResortPage::OnEnChangeResortName()
{
	EnableControls();
}
