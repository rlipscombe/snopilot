// FilePropertiesDevicePage.cpp : implementation file
//

#include "stdafx.h"
#include "FilePropertiesDevicePage.h"
#include "garmin/garmin_packet_ids.h"
#include "utf8.h"

// CFilePropertiesDevicePage dialog

IMPLEMENT_DYNAMIC(CFilePropertiesDevicePage, CPropertyPage)

CFilePropertiesDevicePage::CFilePropertiesDevicePage(const Product_Data_Type *productData)
	: CPropertyPage(CFilePropertiesDevicePage::IDD), m_productData(productData)
{
}

CFilePropertiesDevicePage::~CFilePropertiesDevicePage()
{
}

void CFilePropertiesDevicePage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ADDITIONAL_INFORMATION, m_additionalInformation);
}


BEGIN_MESSAGE_MAP(CFilePropertiesDevicePage, CPropertyPage)
END_MESSAGE_MAP()


// CFilePropertiesDevicePage message handlers

BOOL CFilePropertiesDevicePage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	SetDlgItemInt(IDC_PRODUCT_ID, m_productData->product_id, FALSE);
	
	CString softwareVersion;
	softwareVersion.Format(_T("%d.%02d"), m_productData->software_version / 100, m_productData->software_version % 100);
	SetDlgItemText(IDC_SOFTWARE_VERSION, softwareVersion);

	SetDlgItemText(IDC_PRODUCT_DESCRIPTION, ConvertASCIIToSystem(m_productData->product_description).c_str());

	for (size_t i = 0; i < m_productData->additional_strings.size(); ++i)
	{
		m_additionalInformation.AddString(ConvertASCIIToSystem(m_productData->additional_strings[i]).c_str());
	}

	return TRUE;
}
