#include "StdAfx.h"
#include "SnoTrackFileReader.h"
#include "SnoTrackFileHelper.h"
#include "SnoTrackFileMagic.h"
#include "garmin/garmin_packet_ids.h"
#include "win32/filetime_helpers.h"

void CSnoTrackFileReader::ReadFile(CFile *pFile, CSnoTrackFileReaderCallback *pCallback)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	ASSERT(pFile);
	CFileHelper helper(pFile);

	// Check the magic.
	ASSERT(sizeof(magic_v0_0_27) == 8);	// Must be 8 bytes.
	ASSERT(sizeof(magic_v0_0_28) == 8);
	ASSERT(sizeof(magic_v0_0_34) == 8);
	ASSERT(sizeof(magic_v0_0_36) == 8);
	ASSERT(sizeof(magic_v0_0_58) == 8);

	unsigned char magic[8];
	helper.ReadMagic(magic);

	int fileFormat = 0;
	if (memcmp(magic, magic_v0_0_28, 8) == 0)
	{
		// Then it's a v0.0.28 file.  See http://extranet.snopilot.com/node/view/63
		fileFormat = 28;
	}
	else if (memcmp(magic, magic_v0_0_27, 8) == 0)
	{
		// Then it's a v0.0.27 file.  See http://extranet.snopilot.com/node/view/61
		fileFormat = 27;
	}
	else if (memcmp(magic, magic_v0_0_34, 8) == 0)
	{
		fileFormat = 34;
	}
	else if (memcmp(magic, magic_v0_0_36, 8) == 0)
	{
		fileFormat = 36;
	}
	else if (memcmp(magic, magic_v0_0_58, 8) == 0)
	{
		fileFormat = 58;
	}
	else
		AfxThrowArchiveException(CArchiveException::badIndex, pFile->GetFileName());

	// v0.0.28 begins with the resort file name and the user comments.
	if (fileFormat >= 28)
	{
		// string1 resort
		tstring resortFileName = ConvertUTF8ToPathName(helper.ReadString1());
		pCallback->SetResortFileName(resortFileName);

		// string2 comments
		utf8_string strComments = helper.ReadString2();
		pCallback->SetComments(strComments);
	}
	else
	{
		pCallback->SetResortFileName(_T(""));
		pCallback->SetComments(utf8_string());
	}

	// v0.0.34 has the custom resort name and country next.
	if (fileFormat >= 34)
	{
		utf8_string strCustomResortName = helper.ReadString1();
		pCallback->SetCustomResortName(strCustomResortName);

		utf8_string unused = helper.ReadString1();
	}
	else
	{
		pCallback->SetCustomResortName("");
	}

	// v0.0.36 has the GPS units used after that.
	if (fileFormat >= 36)
	{
		Product_Data_Type productData;

		productData.product_id = helper.ReadInteger();
		productData.software_version = helper.ReadInteger();

		productData.product_description = helper.ReadString1();
		
		DWORD additional_string_count = helper.ReadInteger();
		productData.additional_strings.reserve(additional_string_count);
		for (unsigned i = 0; i < additional_string_count; ++i)
		{
			std::string additional_string = helper.ReadString1();
			productData.additional_strings.push_back(additional_string);
		}

		pCallback->SetProductData(productData);
	}

	// But after that, it's identical to v0.0.27:
	// Load the tracklog.
	DWORD dwTracklogCount = helper.ReadInteger();
	pCallback->SetTracklogSize(dwTracklogCount);

	for (DWORD i = 0; i < dwTracklogCount; ++i)
	{
		FILETIME timestamp;

		if (fileFormat >= 58)
		{
			timestamp.dwLowDateTime = helper.ReadInteger();
			timestamp.dwHighDateTime = helper.ReadInteger();
		}
		else
		{
			DWORD unix_timestamp = helper.ReadInteger();
			timestamp = FileTimeFromUnixTime(unix_timestamp);
		}

		utf8_string trk_ident = helper.ReadString1();
		bool is_new = helper.ReadBool();

		// The longitude, latitude and elevation.  Saved as IEEE format doubles.
		double lon_deg = helper.ReadDouble();
		double lat_deg = helper.ReadDouble();
		double gps_elev = helper.ReadDouble();

		pCallback->AddPositionFix(timestamp, trk_ident, is_new, lon_deg, lat_deg, gps_elev);
	}

	// Immediately after that comes the waypoint information, similarly formatted to the tracklog info.
	DWORD dwWaypointCount = helper.ReadInteger();
	pCallback->SetWaypointCount(dwWaypointCount);

	for (DWORD i = 0; i < dwWaypointCount; ++i)
	{
		utf8_string label = helper.ReadString1();
		double lon_deg = helper.ReadDouble();
		double lat_deg = helper.ReadDouble();

		pCallback->AddWaypoint(label, lon_deg, lat_deg);
	}
}

