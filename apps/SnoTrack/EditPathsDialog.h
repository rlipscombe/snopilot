#pragma once
#include "resource.h"
#include "HotspotPath.h"

// CEditPathsDialog dialog
class CEditPathsDialog : public CDialog
{
	HotspotPathCollection m_paths;
	CListCtrl m_listCtrl;

	DECLARE_DYNAMIC(CEditPathsDialog)

public:
	CEditPathsDialog(const HotspotPathCollection &paths, CWnd* pParent = NULL);   // standard constructor
	virtual ~CEditPathsDialog();

	HotspotPathCollection GetHotspotPaths() const { return m_paths; }

	virtual BOOL OnInitDialog();

// Dialog Data
	enum { IDD = IDD_EDIT_PATHS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	afx_msg void OnLvnItemchangedListctrl(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedEditPath();
	afx_msg void OnBnClickedDeletePath();
	afx_msg void OnBnClickedNewPath();
	afx_msg void OnNMDblclkListctrl(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDestroy();

	DECLARE_MESSAGE_MAP()

private:
	void EnableControls();
	void PopulateListCtrl();
	int InsertListItem(const HotspotPath &path);
	void UpdateItem(int nItem);
};
