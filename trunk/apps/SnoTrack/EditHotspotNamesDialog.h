#pragma once
#include "resource.h"
#include "HotspotNames.h"
#include "HotspotGrid.h"

// CEditHotspotNamesDialog dialog
struct HotspotDetails
{
	int hotspotID;
	CString hotspotName;
};

typedef std::vector< HotspotDetails > HotspotDetailsVec;

class CEditHotspotNamesDialog : public CDialog
{
	CListCtrl m_listCtrl;

	HotspotDetailsVec m_hotspotDetailsVec;

	DECLARE_DYNAMIC(CEditHotspotNamesDialog)

public:
	CEditHotspotNamesDialog(const HotspotGrid *grid, const HotspotNames &hotspotNames, CWnd* pParent = NULL);   // standard constructor
	virtual ~CEditHotspotNamesDialog();

	virtual BOOL OnInitDialog();

	HotspotNames GetHotspotNames() const;

// Dialog Data
	enum { IDD = IDD_EDIT_HOTSPOT_NAMES };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	afx_msg void OnLvnItemchangedListctrl(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedEditName();
	afx_msg void OnNMDblclkListctrl(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDestroy();

	DECLARE_MESSAGE_MAP()

private:
	void EnableControls();
	void PopulateListCtrl();
	int InsertListItem(const HotspotDetails &d);
	void UpdateItem(int nItem);
};
