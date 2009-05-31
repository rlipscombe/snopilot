// FilePropertiesResortPage.cpp : implementation file
//

#include "stdafx.h"
#include "FilePropertiesResortPage.h"

// CFilePropertiesResortPage dialog

IMPLEMENT_DYNAMIC(CFilePropertiesResortPage, CPropertyPage)

CFilePropertiesResortPage::CFilePropertiesResortPage(CString resortFileName, CString resortName, CString resortTimeZoneName)
	: CPropertyPage(CFilePropertiesResortPage::IDD),
		m_resortFileName(resortFileName), m_resortName(resortName), m_resortTimeZoneName(resortTimeZoneName)
{
}

CFilePropertiesResortPage::~CFilePropertiesResortPage()
{
}

void CFilePropertiesResortPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CFilePropertiesResortPage, CPropertyPage)
END_MESSAGE_MAP()


// CFilePropertiesResortPage message handlers

BOOL CFilePropertiesResortPage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	SetDlgItemText(IDC_RESORT_FILE, m_resortFileName);
	SetDlgItemText(IDC_RESORT_NAME, m_resortName);
	SetDlgItemText(IDC_RESORT_TIMEZONE, m_resortTimeZoneName);

	return TRUE;
}
