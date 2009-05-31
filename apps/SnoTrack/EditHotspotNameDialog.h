#pragma once
#include "resource.h"

// CEditHotspotNameDialog dialog
class CEditHotspotNameDialog : public CDialog
{
	int m_hotspotID;
	CString m_hotspotName;

	DECLARE_DYNAMIC(CEditHotspotNameDialog)

public:
	CEditHotspotNameDialog(int hotspotID, CString hotspotName, CWnd* pParent = NULL);   // standard constructor
	virtual ~CEditHotspotNameDialog();

	virtual BOOL OnInitDialog();

	CString GetHotspotName() const { return m_hotspotName; }

// Dialog Data
	enum { IDD = IDD_DIALOG1 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnOK();

	DECLARE_MESSAGE_MAP()
};
