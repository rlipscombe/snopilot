#pragma once
#if defined(SNOTRACK_EXPORT)
 #define SNOTRACK_API __declspec(dllexport)
#else
 #define SNOTRACK_API __declspec(dllimport)
#endif

#include "utf8.h"

class CSnoTrackFileReaderCallback;

class SNOTRACK_API CSnoTrackFileReader
{
public:
	void ReadFile(CFile *pFile, CSnoTrackFileReaderCallback *pCallback);
};

struct Product_Data_Type;

class CSnoTrackFileReaderCallback
{
public:
	virtual void SetProductData(const Product_Data_Type &productData) = 0;

	virtual void SetComments(const utf8_string &comments) = 0;
	virtual void SetResortFileName(const tstring &resortFileName) = 0;

	virtual void SetCustomResortName(const utf8_string &customResortName) = 0;
	
	virtual void SetTracklogSize(DWORD dwTracklogSize) = 0;
	virtual void AddPositionFix(FILETIME unix_timestamp, const utf8_string &trk_ident, bool is_new, double lon_deg, double lat_deg, double gps_elev) = 0;

	virtual void SetWaypointCount(DWORD dwWaypointCount) = 0;
	virtual void AddWaypoint(const utf8_string &label, double lon_deg, double lat_deg) = 0;
};
