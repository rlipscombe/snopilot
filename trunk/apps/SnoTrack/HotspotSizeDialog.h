#pragma once
#include "resource.h"

#if defined(ENABLE_DESIGN_MODE)

// CHotspotSizeDialog dialog

class CHotspotSizeDialog : public CDialog
{
	int m_hotspotSize;

	DECLARE_DYNAMIC(CHotspotSizeDialog)

public:
	CHotspotSizeDialog(int hotspotSize, CWnd* pParent = NULL);   // standard constructor
	virtual ~CHotspotSizeDialog();

	int GetHotspotSize() const { return m_hotspotSize; }

// Dialog Data
	enum { IDD = IDD_SELECT_HOTSPOT_SIZE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
};

#endif // ENABLE_DESIGN_MODE
