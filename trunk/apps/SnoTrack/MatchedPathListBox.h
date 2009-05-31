#pragma once
#include "OwnerDrawListBox.h"
#include "HotspotPath.h"

// CMatchedPathListBox
class CMatchedPathListBox : public COwnerDrawListBox
{
	CFont m_font;
	CImageList *m_pImageList;
	TIME_ZONE_INFORMATION m_tzi;

	struct MatchEvent
	{
		CString description;
		FILETIME startTime;
		FILETIME endTime;
		double distance;
		int imageIndex;
	};

	DECLARE_DYNAMIC(CMatchedPathListBox)

public:
	CMatchedPathListBox();
	virtual ~CMatchedPathListBox();

	CImageList *SetImageList(CImageList *piml);
	
	void SetTimeZoneInformation(const TIME_ZONE_INFORMATION *pTimeZoneInformation)
	{
		m_tzi = *pTimeZoneInformation;
	}

	void AddEvent(const CString &description, FILETIME startTime, FILETIME endTime, double distance, int imageIndex);
	bool GetEvent(int nIndex, FILETIME *startTime, FILETIME *endTime);

protected:
	virtual void PreSubclassWindow();

	virtual void DoDrawItem(CDC *pDC, const void *itemData, const CRect &itemRect, DWORD itemState);
	virtual void MeasureItem(LPMEASUREITEMSTRUCT measureItem);
	virtual int CompareItem(LPCOMPAREITEMSTRUCT compareItem);
	virtual void DeleteItem(LPDELETEITEMSTRUCT deleteItem);

protected:
	afx_msg void OnSize(UINT nType, int cx, int cy);

	DECLARE_MESSAGE_MAP()

private:
	CString FormatPathTime(FILETIME ft);
};
