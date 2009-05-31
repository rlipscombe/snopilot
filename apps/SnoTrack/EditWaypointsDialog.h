#pragma once
#include "resource.h"
#include "Waypoint.h"

class Projection;

// CEditWaypointsDialog dialog
class CEditWaypointsDialog : public CDialog
{
	CListCtrl m_listCtrl;
	WaypointCollection m_waypoints;
	Projection *m_proj;

	DECLARE_DYNAMIC(CEditWaypointsDialog)

public:
	CEditWaypointsDialog(const WaypointCollection &waypoints, Projection *proj, CWnd* pParent = NULL);   // standard constructor
	virtual ~CEditWaypointsDialog();

	virtual BOOL OnInitDialog();

	void GetWaypoints(WaypointCollection *waypoints) const
	{
		*waypoints = m_waypoints;
	}

// Dialog Data
	enum { IDD = IDD_EDIT_WAYPOINTS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	afx_msg void OnDestroy();
	afx_msg void OnBnClickedDelete();
	afx_msg void OnBnClickedEdit();
	afx_msg void OnBnClickedNew();
	afx_msg void OnNMDblclkListctrl(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnItemchangedListctrl(NMHDR *pNMHDR, LRESULT *pResult);

	DECLARE_MESSAGE_MAP()

private:
	void PopulateListCtrl();
	int InsertListItem(const Waypoint &waypoint);
	void UpdateItem(int nItem);
	void EnableControls();
};
