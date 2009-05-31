// FilePropertiesDialog.cpp : implementation file
//

#include "stdafx.h"
#include "FilePropertiesDialog.h"
#include "FilePropertiesResortPage.h"
#include "FilePropertiesTracklogPage.h"
#include "FilePropertiesDevicePage.h"
#include "garmin/garmin_packet_ids.h"

// CFilePropertiesDialog

IMPLEMENT_DYNAMIC(CFilePropertiesDialog, CPropertySheet)

CFilePropertiesDialog::CFilePropertiesDialog(int tracklogCount, int waypointCount, GridBounds gridBounds, double minElevation, double maxElevation, FILETIME minTimestamp, FILETIME maxTimestamp, const TIME_ZONE_INFORMATION *pTimeZoneInformation, CString resortFileName, CString resortName, CString resortTimeZoneName, const Product_Data_Type *productData, CWnd* pParentWnd, UINT iSelectPage)
	: CPropertySheet(IDS_PROPERTIES_CAPTION, pParentWnd, iSelectPage)
{
	AddPage(NEW CFilePropertiesResortPage(resortFileName, resortName, resortTimeZoneName));
	AddPage(NEW CFilePropertiesTracklogPage(tracklogCount, waypointCount, gridBounds, minElevation, maxElevation, minTimestamp, maxTimestamp, pTimeZoneInformation));
	if (productData->product_id != 0)
		AddPage(NEW CFilePropertiesDevicePage(productData));
}

CFilePropertiesDialog::~CFilePropertiesDialog()
{
	for (int i = 0; i < GetPageCount(); ++i)
	{
		CPropertyPage *pPage = GetPage(i);

		delete pPage;
	}
}

BEGIN_MESSAGE_MAP(CFilePropertiesDialog, CPropertySheet)
END_MESSAGE_MAP()


// CFilePropertiesDialog message handlers
