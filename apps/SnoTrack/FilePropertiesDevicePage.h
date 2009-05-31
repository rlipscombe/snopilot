#pragma once
#include "resource.h"
#include "afxwin.h"

struct Product_Data_Type;

// CFilePropertiesDevicePage dialog
class CFilePropertiesDevicePage : public CPropertyPage
{
	const Product_Data_Type *m_productData;

	DECLARE_DYNAMIC(CFilePropertiesDevicePage)

public:
	CFilePropertiesDevicePage(const Product_Data_Type *productData);
	virtual ~CFilePropertiesDevicePage();

	virtual BOOL OnInitDialog();

// Dialog Data
	enum { IDD = IDD_FILE_PROPERTIES_DEVICE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	CListBox m_additionalInformation;
};
