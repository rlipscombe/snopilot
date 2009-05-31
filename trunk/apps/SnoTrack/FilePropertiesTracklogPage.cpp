#include "stdafx.h"
#include "FilePropertiesTracklogPage.h"
#include "win32/TimeZoneInformation.h"
#include "LatLon.h"

CString FormatElevation(double elev_m)
{
	CString s;
	s.Format("%f m", elev_m);
	return s;
}

// CFilePropertiesTracklogPage dialog

IMPLEMENT_DYNAMIC(CFilePropertiesTracklogPage, CPropertyPage)

CFilePropertiesTracklogPage::CFilePropertiesTracklogPage(int tracklogCount, int waypointCount, GridBounds gridBounds,
														 double minElevation, double maxElevation,
														 FILETIME minTimestamp, FILETIME maxTimestamp, const TIME_ZONE_INFORMATION *pTimeZoneInformation)
	: CPropertyPage(CFilePropertiesTracklogPage::IDD),
		m_tracklogCount(tracklogCount), m_waypointCount(waypointCount), m_gridBounds(gridBounds),
		m_minElevation(minElevation), m_maxElevation(maxElevation),
		m_minTimestamp(minTimestamp), m_maxTimestamp(maxTimestamp), m_tzi(*pTimeZoneInformation)
{
}

CFilePropertiesTracklogPage::~CFilePropertiesTracklogPage()
{
}

void CFilePropertiesTracklogPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CFilePropertiesTracklogPage, CPropertyPage)
END_MESSAGE_MAP()

BOOL CFilePropertiesTracklogPage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	SetDlgItemInt(IDC_TRACKLOG_COUNT, m_tracklogCount);
	SetDlgItemInt(IDC_WAYPOINT_COUNT, m_waypointCount);

	if (m_tracklogCount)
	{
		SetDlgItemText(IDC_MIN_LONGITUDE, FormatLongitude(m_gridBounds.m_minLongitude, FORMAT_LON_GARMIN));
		SetDlgItemText(IDC_MAX_LONGITUDE, FormatLongitude(m_gridBounds.m_maxLongitude, FORMAT_LON_GARMIN));

		SetDlgItemText(IDC_MIN_LATITUDE, FormatLatitude(m_gridBounds.m_minLatitude, FORMAT_LAT_GARMIN));
		SetDlgItemText(IDC_MAX_LATITUDE, FormatLatitude(m_gridBounds.m_maxLatitude, FORMAT_LAT_GARMIN));

		SetDlgItemText(IDC_MIN_ELEVATION, FormatElevation(m_minElevation));
		SetDlgItemText(IDC_MAX_ELEVATION, FormatElevation(m_maxElevation));

		FillTimestampField(IDC_MIN_TIMESTAMP, m_minTimestamp);
		FillTimestampField(IDC_MAX_TIMESTAMP, m_maxTimestamp);
	}
	else
	{
		GetDlgItem(IDC_MIN_LONGITUDE)->EnableWindow(FALSE);
		GetDlgItem(IDC_MAX_LONGITUDE)->EnableWindow(FALSE);

		GetDlgItem(IDC_MIN_LATITUDE)->EnableWindow(FALSE);
		GetDlgItem(IDC_MAX_LATITUDE)->EnableWindow(FALSE);

		GetDlgItem(IDC_MIN_ELEVATION)->EnableWindow(FALSE);
		GetDlgItem(IDC_MAX_ELEVATION)->EnableWindow(FALSE);

		GetDlgItem(IDC_MIN_TIMESTAMP)->EnableWindow(FALSE);
		GetDlgItem(IDC_MAX_TIMESTAMP)->EnableWindow(FALSE);
	}

	return TRUE;
}

void CFilePropertiesTracklogPage::FillTimestampField(UINT nIDC, FILETIME timestamp)
{
	CString str = FormatUtcFileTimeInTzSpecificLocalTime(timestamp, &m_tzi);

	SetDlgItemText(nIDC, str);
}
