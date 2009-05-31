#pragma once
#include "SnoTrackDLL/SnoTrackFileReader.h"
#include "garmin/garmin_packet_ids.h"

class SummaryReaderCallback : public CSnoTrackFileReaderCallback
{
	Product_Data_Type m_productData;

	utf8_string m_comments;

	tstring m_resortFileName;
	utf8_string m_customResortName;
	utf8_string m_customResortCountry;

	DWORD m_tracklogSize;
	DWORD m_waypointCount;

public:
	SummaryReaderCallback()
		: m_tracklogSize(0), m_waypointCount(0)
	{
	}

	void GetProductData(Product_Data_Type *productData) const
	{
		*productData = m_productData;
	}

	utf8_string GetComments() const
	{
		return m_comments;
	}

	tstring GetResortFileName() const
	{
		return m_resortFileName;
	}

	utf8_string GetCustomResortName() const
	{
		return m_customResortName;
	}

	utf8_string GetCustomResortCountry() const
	{
		return m_customResortCountry;
	}

	CString GetTracklogSizeStr() const
	{
		CString s;
		s.Format(_T("%d"), m_tracklogSize);
		return s;
	}

	CString GetWaypointCountStr() const
	{
		CString s;
		s.Format(_T("%d"), m_waypointCount);
		return s;
	}

// CSnoTrackFileReaderCallback
public:
	virtual void SetProductData(const Product_Data_Type &productData)
	{
		m_productData = productData;
	}

	virtual void SetComments(const utf8_string &comments)
	{
		m_comments = comments;
	}

	virtual void SetResortFileName(const tstring &resortFileName)
	{
		m_resortFileName = resortFileName;
	}
	
	virtual void SetCustomResortName(const utf8_string &customResortName)
	{
		m_customResortName = customResortName;
	}

	virtual void SetCustomResortCountry(const utf8_string &customResortCountry)
	{
		m_customResortCountry = customResortCountry;
	}
	
	virtual void SetTracklogSize(DWORD dwTracklogSize)
	{
		m_tracklogSize = dwTracklogSize;
	}

	virtual void AddPositionFix(FILETIME timestamp, const utf8_string &ctrk_ident, bool is_new, double lon_deg, double lat_deg, double gps_elev)
	{
	}

	virtual void SetWaypointCount(DWORD dwWaypointCount)
	{
		m_waypointCount = dwWaypointCount;
	}

	virtual void AddWaypoint(const utf8_string &clabel, double lon_deg, double lat_deg)
	{
	}
};

