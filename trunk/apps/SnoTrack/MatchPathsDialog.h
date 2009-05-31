#pragma once
#include "resource.h"
#include <vector>
#include "utf8.h"

class HotspotPath;

// CMatchPathsDialog dialog

class CMatchPathsDialog : public CDialog
{
	TIME_ZONE_INFORMATION m_tzi;

	struct MatchEvent
	{
		CString description;
		FILETIME startTime;
		FILETIME endTime;
		int imageIndex;
	};

	typedef std::vector< MatchEvent * > EventCollection;
	EventCollection m_events;

	CListCtrl m_listCtrl;

	DECLARE_DYNAMIC(CMatchPathsDialog)

public:
	CMatchPathsDialog(const TIME_ZONE_INFORMATION *pTimeZoneInformation, CWnd* pParent = NULL);   // standard constructor
	virtual ~CMatchPathsDialog();

	virtual BOOL OnInitDialog();

	void AddMatchedPath(const HotspotPath &path, FILETIME pathEntryTime, FILETIME pathExitTime);
	void AddUnmatchedHotspot(int id, FILETIME entryTime, FILETIME exitTime, int imageIndex);

// Dialog Data
	enum { IDD = IDD_MATCH_PATHS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
	afx_msg void OnLvnGetdispinfoListctrl(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnDeleteitemListctrl(NMHDR *pNMHDR, LRESULT *pResult);

	DECLARE_MESSAGE_MAP()

private:
	void AddEvent(const CString &description, FILETIME startTime, FILETIME endTime, int imageIndex);

	static int CALLBACK CompareStartTimeFunction(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
};
