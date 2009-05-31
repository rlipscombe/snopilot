#include "StdAfx.h"
#include "SnoTrackFileWriter.h"
#include "SnoTrackFileHelper.h"
#include "SnoTrackFileMagic.h"
#include "garmin/garmin_packet_ids.h"

class TracklogWriter : public CSnoTrackFileWriterCallback::TracklogWriterCallback
{
	CFileHelper *m_pHelper;
	int m_fileFormat;

public:
	TracklogWriter(CFileHelper *pHelper, int fileFormat)
		: m_pHelper(pHelper), m_fileFormat(fileFormat)
	{
	}

	virtual void WritePositionFix(FILETIME timestamp, const char *trk_ident, bool is_new, double lon_deg, double lat_deg, double gps_elev)
	{
		// The timestamp:
		if (m_fileFormat >= 58)
		{
			m_pHelper->WriteInteger(timestamp.dwLowDateTime);
			m_pHelper->WriteInteger(timestamp.dwHighDateTime);
		}
		else
		{
			ASSERT(false);
			// It's a Unix time_t.  We store it as a 32-bit little-endian integer.
//			m_pHelper->WriteInteger((DWORD)unix_timestamp);
		}

		// The track ID: It's a string.
		m_pHelper->WriteString1(trk_ident);

		// Is this a new track?  Saved as a single byte.
		m_pHelper->WriteBool(is_new);

		// The longitude, latitude and elevation.  Saved as IEEE format doubles.
		m_pHelper->WriteDouble(lon_deg);
		m_pHelper->WriteDouble(lat_deg);
		m_pHelper->WriteDouble(gps_elev);
	}
};

void CSnoTrackFileWriter::WriteFile(CFile *pFile, CSnoTrackFileWriterCallback *pCallback)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	ASSERT(pFile);
	CFileHelper helper(pFile);

	ASSERT(sizeof(magic_v0_0_58) == 8);
	helper.WriteMagic(magic_v0_0_58);

	// Quietly convert to the current format.
	const int fileFormat = 58;

	// v0.0.28 begins with the resort file name and the user comments.
	if (fileFormat >= 28)
	{
		// string1 resort
		tstring resortFileName = pCallback->GetResortFileName();
		helper.WriteString1(ConvertPathNameToUTF8(resortFileName));

		// string2 comments
		utf8_string strComments = pCallback->GetComments();
		helper.WriteString2(strComments);
	}

	// v0.0.34 has the custom resort details after that.  Regardless of whether the resort
	// file name was filled in correctly or not.
	if (fileFormat >= 34)
	{
		utf8_string strCustomResortName = pCallback->GetCustomResortName();
		helper.WriteString1(strCustomResortName);

		utf8_string unused;
		helper.WriteString1(unused);
	}
	
	// v0.0.36 has the GPS units used after that.
	if (fileFormat >= 36)
	{
		Product_Data_Type productData;
		pCallback->GetProductData(&productData);

		helper.WriteInteger(productData.product_id);
		helper.WriteInteger(productData.software_version);

		helper.WriteString1(productData.product_description.c_str());
		
		helper.WriteInteger((DWORD)productData.additional_strings.size());
		for (unsigned i = 0; i < productData.additional_strings.size(); ++i)
			helper.WriteString1(productData.additional_strings[i].c_str());
	}

	// But after that, it's identical to v0.0.27:

	// The next block in the file is the tracklog.
	// It starts with the count of points, stored as a little-endian 32-bit integer.
	DWORD dwTracklogSize = pCallback->GetTracklogSize();
	helper.WriteInteger(dwTracklogSize);

	TracklogWriter tracklogWriter(&helper, fileFormat);
	pCallback->GetTracklog(&tracklogWriter);

	DWORD dwWaypointCount = pCallback->GetWaypointCount();
	helper.WriteInteger(dwWaypointCount);

	class WaypointWriter : public CSnoTrackFileWriterCallback::WaypointWriterCallback
	{
		CFileHelper *m_pHelper;

	public:
		WaypointWriter(CFileHelper *pHelper)
			: m_pHelper(pHelper)
		{
		}

		virtual void WriteWaypoint(const char *label, double lon_deg, double lat_deg)
		{
			m_pHelper->WriteString1(label);
			m_pHelper->WriteDouble(lon_deg);
			m_pHelper->WriteDouble(lat_deg);
		}
	};

	WaypointWriter waypointWriter(&helper);
	pCallback->GetWaypoints(&waypointWriter);
}
