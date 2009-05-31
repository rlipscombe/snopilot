#pragma once
#include "resource.h"
#include "HotspotPath.h"
#include "afxcmn.h"

// CEditPathDialog dialog
class CEditPathDialog : public CDialog
{
	HotspotPath m_path;
	CListCtrl m_listCtrl;

	DECLARE_DYNAMIC(CEditPathDialog)

public:
	CEditPathDialog(const HotspotPath &path, CWnd* pParent = NULL);
	virtual ~CEditPathDialog();

	virtual BOOL OnInitDialog();

	HotspotPath GetHotspotPath() const { return m_path; }

// Dialog Data
	enum { IDD = IDD_EDIT_PATH };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual void OnOK();

	afx_msg void OnDestroy();

	DECLARE_MESSAGE_MAP()
};
