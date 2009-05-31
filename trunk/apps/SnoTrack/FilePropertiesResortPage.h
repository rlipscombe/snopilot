#pragma once
#include "resource.h"

// CFilePropertiesResortPage dialog
class CFilePropertiesResortPage : public CPropertyPage
{
	CString m_resortFileName;
	CString m_resortName;
	CString m_resortTimeZoneName;

	DECLARE_DYNAMIC(CFilePropertiesResortPage)

public:
	CFilePropertiesResortPage(CString resortFileName, CString resortName, CString resortTimeZoneName);
	virtual ~CFilePropertiesResortPage();

	virtual BOOL OnInitDialog();

// Dialog Data
	enum { IDD = IDD_FILE_PROPERTIES_RESORT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
