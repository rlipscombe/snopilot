#pragma once
#include "resource.h"

#include "PositionFix.h"
#include "HotspotGrid.h"
#include "afxcmn.h"

// CWalkHotspotsDialog dialog
class CWalkHotspotsDialog : public CDialog
{
	TIME_ZONE_INFORMATION m_tzi;
	const PositionFixCollection &m_positions;
	const HotspotGrid *m_pHotspotGrid;

	CListCtrl m_listCtrl;

	DECLARE_DYNAMIC(CWalkHotspotsDialog)

public:
	CWalkHotspotsDialog(const TIME_ZONE_INFORMATION *pTimeZoneInformation, const PositionFixCollection &positions, const HotspotGrid *pHotspotGrid, CWnd* pParent = NULL);   // standard constructor
	virtual ~CWalkHotspotsDialog();

	virtual BOOL OnInitDialog();

// Dialog Data
	enum { IDD = IDD_WALK_HOTSPOTS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	int InsertListItem(int hotspotId, FILETIME entryTime, FILETIME exitTime, int iImage);
public:
	afx_msg void OnDestroy();
};
