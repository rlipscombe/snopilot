#pragma once
#include "resource.h"

// CSelectHotspotIdDialog dialog

class CSelectHotspotIdDialog : public CDialog
{
	int m_currentHotspotID;
	int m_nextHotspotID;

	DECLARE_DYNAMIC(CSelectHotspotIdDialog)

public:
	CSelectHotspotIdDialog(int currentHotspotID, int nextHotspotID, CWnd* pParent = NULL);
	virtual ~CSelectHotspotIdDialog();

	virtual BOOL OnInitDialog();

// Dialog Data
	enum { IDD = IDD_SELECT_HOTSPOT_ID };

	int GetNextID() const { return m_nextHotspotID; }

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnOK();

	DECLARE_MESSAGE_MAP()
};
