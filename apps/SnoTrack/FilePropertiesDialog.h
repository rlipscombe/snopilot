#pragma once

struct Product_Data_Type;
struct GridBounds;

// CFilePropertiesDialog
class CFilePropertiesDialog : public CPropertySheet
{
	DECLARE_DYNAMIC(CFilePropertiesDialog)

public:
	CFilePropertiesDialog(int tracklogCount, int waypointCount, GridBounds gridBounds, double minElevation, double maxElevation, FILETIME minTimestamp, FILETIME maxTimestamp, const TIME_ZONE_INFORMATION *pTimeZoneInformation,
							CString resortFileName, CString resortName, CString resortTimeZoneName,
							const Product_Data_Type *productData,
							CWnd* pParentWnd = NULL, UINT iSelectPage = 0);

	virtual ~CFilePropertiesDialog();

protected:
	DECLARE_MESSAGE_MAP()
};
