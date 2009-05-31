#pragma once
#include "resource.h"
#include "GridBounds.h"

// CFilePropertiesTracklogPage dialog
class CFilePropertiesTracklogPage : public CPropertyPage
{
	int m_tracklogCount;
	int m_waypointCount;
	GridBounds m_gridBounds;

	double m_minElevation;
	double m_maxElevation;

	FILETIME m_minTimestamp;
	FILETIME m_maxTimestamp;
	TIME_ZONE_INFORMATION m_tzi;

	DECLARE_DYNAMIC(CFilePropertiesTracklogPage)

public:
	CFilePropertiesTracklogPage(int tracklogCount, int waypointCount, GridBounds gridBounds,
		double minElevation, double maxElevation,
		FILETIME minTimestamp, FILETIME maxTimestamp, const TIME_ZONE_INFORMATION *pTimeZoneInformation);

	virtual ~CFilePropertiesTracklogPage();

	virtual BOOL OnInitDialog();

// Dialog Data
	enum { IDD = IDD_TRACKLOG_STATS_PAGE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	void FillTimestampField(UINT nIDC, FILETIME timestamp);
};
