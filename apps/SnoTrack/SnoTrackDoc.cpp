// SnoTrackDoc.cpp : implementation of the CSnoTrackDoc class
//

#include "stdafx.h"
#include "SnoTrackDoc.h"

#include "ImportSerialWizard.h"
#include "ImportSerialSettings.h"

#include "ImportFileProgressDialog.h"
#include "PositionCollector.h"

#include "line_collector.h"
#include "garmin/garmin_time.h"
#include "garmin/garmin_semicircles.h"
#include "progress.h"
#include <algorithm>
#include "win32/format_error.h"
#include "win32/ApplicationProfile.h"

#include "NewFileWizard.h"
#include "NewFileSettings.h"
#include "FilePropertiesDialog.h"

#include "nmea_sentence_collector.h"

#include "DefineHotspotGridDialog.h"
#include "SnoTrackDLL/SnoTrackFileHelper.h"
#include "win32/filetime_helpers.h"

#include "WalkHotspotsDialog.h"
#include "win32/TimeZoneInformation.h"
#include "SelectHotspotIdDialog.h"
#include "EditPathsDialog.h"
#include "MatchPathsDialog.h"
#include "HotspotVisit.h"

#include "EditLiftsDialog.h"
#include "EditWaypointsDialog.h"
#include "EditHotspotNamesDialog.h"

#include "SnoLogCollector.h"
#include "LatLon.h"
#include ".\snotrackdoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CSnoTrackDoc
IMPLEMENT_DYNCREATE(CSnoTrackDoc, CDocument)

BEGIN_MESSAGE_MAP(CSnoTrackDoc, CDocument)
	ON_COMMAND(ID_IMPORT_SERIAL, OnImportSerial)
	ON_COMMAND(ID_IMPORT_TRACKLOGFROMFILE, OnImportTracklogFromFile)
	ON_COMMAND(ID_FILE_PROPERTIES, OnFileProperties)
	ON_COMMAND(ID_IMPORT_WAYPOINTSFROMFILE, OnImportWaypointsFromFile)
	ON_COMMAND(ID_FILE_UPLOADFILE, OnFileUploadfile)
	ON_COMMAND(ID_EXPORT_TRACKLOGTOFILE, OnExportTracklogToFile)
	ON_COMMAND(ID_EXPORT_WAYPOINTSTOFILE, OnExportWaypointsToFile)
	ON_COMMAND(ID_IMPORT_NMEAFROMFILE, OnImportNmeafromfile)
	ON_COMMAND(ID_IMPORT_FROMSNOLOGFILE, OnImportFromSnologFile)
#if defined(ENABLE_DESIGN_MODE)
	ON_COMMAND(ID_HOTSPOTS_WALK, OnHotspotsWalk)
	ON_UPDATE_COMMAND_UI(ID_HOTSPOTS_WALK, OnUpdateHotspotsWalk)
	ON_UPDATE_COMMAND_UI(ID_HOTSPOTS_CLEARHOTSPOTS, OnUpdateHotspotsClearhotspots)
	ON_COMMAND(ID_HOTSPOTS_SELECTID, OnHotspotsSelectid)
	ON_UPDATE_COMMAND_UI(ID_HOTSPOTS_SELECTID, OnUpdateHotspotsSelectid)
	ON_COMMAND(ID_HOTSPOTS_EDITPATHS, OnHotspotsEditpaths)
	ON_UPDATE_COMMAND_UI(ID_HOTSPOTS_EDITPATHS, OnUpdateHotspotsEditpaths)
	ON_COMMAND(ID_RESORT_NEW, OnResortNew)
	ON_COMMAND(ID_RESORT_OPEN, OnResortOpen)
	ON_COMMAND(ID_RESORT_SAVEAS, OnResortSaveas)
	ON_COMMAND(ID_RESORT_EDIT_LIFTS, OnResortEditLifts)
	ON_COMMAND(ID_RESORT_EDIT_WAYPOINTS, OnResortEditWaypoints)
	ON_COMMAND(ID_RESORT_RECALCULATEROUTES, OnResortRecalculateroutes)
	ON_COMMAND(ID_RESORT_HOTSPOTNAMES, OnResortHotspotnames)
	ON_UPDATE_COMMAND_UI(ID_RESORT_HOTSPOTNAMES, OnUpdateResortHotspotnames)
#endif
END_MESSAGE_MAP()


// CSnoTrackDoc construction/destruction

CSnoTrackDoc::CSnoTrackDoc()
	: m_min_elev(0.0), m_max_elev(0.0), m_hotspotGrid(NULL), m_currentHotspotID(0),
		m_dwResortOrientation(0), m_bResortModified(false)
{
    char *pargs[] = {
		"proj=merc", "ellps=WGS84",
	};

	m_proj.Create(COUNTOF(pargs), pargs);

	m_resortTimeZone = _T("GMT Standard Time");

	m_min_timestamp.dwLowDateTime = m_min_timestamp.dwHighDateTime = 0;
	m_max_timestamp.dwLowDateTime = m_max_timestamp.dwHighDateTime = 0;
}

CSnoTrackDoc::~CSnoTrackDoc()
{
	if (m_hotspotGrid)
		delete m_hotspotGrid;
}

BOOL CSnoTrackDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	AfxApplicationProfile appProfile(AfxGetApp());

	NewFileSettings settings;
	settings.Load(&appProfile);
	INT_PTR nResult = CNewFileWizard::DoWizard(&settings);

	if (nResult != ID_WIZFINISH)
		return FALSE;

	settings.Save(&appProfile);
	m_comments = settings.GetComments();
	
	if (settings.IsCustomResort())
	{
		// Custom resort
		m_customResortName = settings.GetCustomResortName();
	}
	else
	{
		OnOpenResort(settings.GetResortPath());
	}

	return TRUE;
}

// CSnoTrackDoc diagnostics

#ifdef _DEBUG
void CSnoTrackDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CSnoTrackDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

tstring CSnoTrackDoc::GetTimeZoneName() const
{
	return m_resortTimeZone;
}

// CSnoTrackDoc commands
void CSnoTrackDoc::OnFileProperties()
{
	TIME_ZONE_INFORMATION tzi;
	GetRegistryTimeZoneInformation(GetTimeZoneName().c_str(), &tzi);
	CFilePropertiesDialog dlg((int)m_positions.size(), (int)m_user_waypoints.size(), m_gridBounds, m_min_elev, m_max_elev, m_min_timestamp, m_max_timestamp, &tzi,
								m_resortFileName.c_str(), ConvertUTF8ToSystem(m_resortName).c_str(), m_resortTimeZone.c_str(),
								&m_productData);
	dlg.DoModal();
}

void CSnoTrackDoc::OnImportSerial()
{
	AfxApplicationProfile appProfile(AfxGetApp());

	ImportSerialSettings settings(&m_proj);
	settings.Load(&appProfile);

	ImportSerialResults results;
	INT_PTR result = CImportSerialWizard::DoWizard(&settings, &results);

	if (result == ID_WIZFINISH)
	{
		settings.Save(&appProfile);

		results.GetProductData(&m_productData);

		results.GetPositions(&m_positions);
		results.GetWaypoints(&m_user_waypoints);

		ProcessWaypointsAndTrackLog(NULL);

		SetModifiedFlag(TRUE);
		UpdateAllViews(NULL, HINT_IMPORT_TRACKLOG|HINT_IMPORT_WAYPOINTS);
	}
}

BOOL DoOpenFileDialog(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszDefExt, LPCTSTR lpszFilter, CString *pathName)
{
	ASSERT(pathName);

	CFileDialog dlg(TRUE, lpszDefExt, NULL, OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST, lpszFilter);

	CString strDirectory = AfxGetApp()->GetProfileString(lpszSection, lpszEntry);
	dlg.GetOFN().lpstrInitialDir = strDirectory;

	INT_PTR nResult = dlg.DoModal();
	if (nResult != IDOK)
		return FALSE;

	CString strPathName = dlg.GetPathName();
	strDirectory = strPathName;
	VERIFY(PathRemoveFileSpec(strDirectory.GetBuffer(MAX_PATH)));
	strDirectory.ReleaseBuffer();

	AfxGetApp()->WriteProfileString(lpszSection, lpszEntry, strDirectory);

	*pathName = strPathName;
	return TRUE;
}

// TODO: Allow multiple selection
void CSnoTrackDoc::OnImportTracklogFromFile()
{
	CString tracklogFile;
	if (!DoOpenFileDialog(_T("Directories"), _T("Tracklog CSV Files"), _T("csv"), _T("Comma Separated Values (*.csv)|*.csv||"), &tracklogFile))
		return;

	CImportFileProgressDialog progress;
	progress.Create();

	TracklogPositionCollector positionCollector(&m_positions, &m_proj);
	ImportPositionsFromFile(tracklogFile, &positionCollector, &progress);
	ProcessWaypointsAndTrackLog(&progress);

	progress.DestroyWindow();

	SetModifiedFlag(TRUE);
	UpdateAllViews(NULL, HINT_IMPORT_TRACKLOG);
}

void CSnoTrackDoc::OnImportWaypointsFromFile()
{
	CString waypointFile;
	if (!DoOpenFileDialog(_T("Directories"), _T("Waypoint CSV Files"), _T("csv"), _T("Comma Separated Values (*.csv)|*.csv||"), &waypointFile))
		return;

	CImportFileProgressDialog progress;
	progress.Create();

	WaypointPositionCollector positionCollector(&m_user_waypoints, &m_proj);
	ImportPositionsFromFile(waypointFile, &positionCollector, &progress);
	ProcessWaypointsAndTrackLog(&progress);

	progress.DestroyWindow();

	SetModifiedFlag(TRUE);
	UpdateAllViews(NULL, HINT_IMPORT_WAYPOINTS);
}

void CSnoTrackDoc::ImportPositionsFromFile(const TCHAR *pathName, LineCollectorCallback *pCallback, Progress *pProgress)
{
	// Open that file, parse it for CSV values.  We discard the first line.
	HANDLE hFile = CreateFile(pathName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		AfxMessageBox(IDS_FAILED_TO_OPEN_TRACKLOG_CSV);
		return;
	}

	DWORD dwFileSize = GetFileSize(hFile, NULL);

	const int bufferSize = 12 * 1024;
	char *pBuffer = (char *)malloc(bufferSize);
	if (!pBuffer)
	{
		AfxMessageBox(IDS_MEMORY_ALLOCATION_FAILED);
		return;
	}

	// We need an hourglass.
	CWaitCursor wait;

	LineCollector lineCollector(pCallback);
	BufferCollector bufferCollector;

	DWORD dwBytesProcessed = 0;
	for (;;)
	{
		DWORD bytesRead;
		BOOL result = ReadFile(hFile, pBuffer, bufferSize, &bytesRead, NULL);
		if (!result)
			break;	// Error
		if (bytesRead == 0)
			break;	// End of file.
        
		bufferCollector.PushData(pBuffer, bytesRead, &lineCollector);
		dwBytesProcessed += bytesRead;

		pProgress->OnProgress(dwBytesProcessed, dwFileSize);
	}

	CloseHandle(hFile);

	bufferCollector.Flush(&lineCollector);
	pProgress->OnComplete();

	free(pBuffer);
}

/** @todo Do this at the same time as we import/load them.
 * The only thing that needs to be done afterwards is sorting them.
 */
void CSnoTrackDoc::ProcessWaypointsAndTrackLog(Progress *pProgress)
{
	std::sort(m_positions.begin(), m_positions.end(), PositionsLessByTime());

	m_min_timestamp.dwHighDateTime = m_min_timestamp.dwLowDateTime = INT_MAX;
	m_max_timestamp.dwHighDateTime = m_max_timestamp.dwLowDateTime = 0;

	m_min_elev = DBL_MAX;
	m_max_elev = -DBL_MAX;

	m_gridBounds.m_minLongitude = DBL_MAX;
	m_gridBounds.m_maxLongitude = -DBL_MAX;

	m_gridBounds.m_minLatitude = DBL_MAX;
	m_gridBounds.m_maxLongitude = -DBL_MAX;

	int position_count = (int)m_positions.size();
	int position_pos = 0;

	for (PositionFixCollection::iterator i = m_positions.begin(); i != m_positions.end(); ++i)
	{
		if (pProgress)
			pProgress->OnProgress(position_pos, position_count);
		++position_pos;

		m_min_timestamp = min(i->GetTimestamp(), m_min_timestamp);
		m_max_timestamp = max(i->GetTimestamp(), m_max_timestamp);

		m_min_elev = min(i->GetGpsElevation(), m_min_elev);

		m_gridBounds.m_minLongitude = min(m_gridBounds.m_minLongitude, i->GetLongitude());
		m_gridBounds.m_maxLongitude = max(m_gridBounds.m_maxLongitude, i->GetLongitude());
		
		m_gridBounds.m_minLatitude = min(m_gridBounds.m_minLatitude, i->GetLatitude());
		m_gridBounds.m_maxLatitude = max(m_gridBounds.m_maxLatitude, i->GetLatitude());

		// Filter out obviously bogus data points, elevation-wise.
		if (i->GetGpsElevation() >= 5000)
		{
//			TRACE("i->GetGpsElevation() = %f\n", i->GetGpsElevation());
		}
		else
			m_max_elev = max(i->GetGpsElevation(), m_max_elev);
	}

#if 0
	// Calculate speed
	if (m_positions.size() >= 2)
	{
		// We'll do this in 3 passes (and optimise it later).
		PositionFixCollection::iterator b = m_positions.begin();
		PositionFix prev_fix = *b++;

		PositionFixCollection::iterator e = m_positions.end();

		int row = 3;
		for (PositionFixCollection::iterator i = b; i != e; ++i)
		{
			PositionFix this_fix = *i;

			double dLat = prev_fix.GetLatitude() - this_fix.GetLatitude();
			double dLon = prev_fix.GetLongitude() - this_fix.GetLongitude();

			const double KM_PER_DEGREE_LAT = 111.323;
			double dist_y = KM_PER_DEGREE_LAT * dLat;

			double lat_radians = (M_PI * this_fix.GetLatitude()) / 180;
			double dist_x = KM_PER_DEGREE_LAT * dLon * cos(lat_radians);

			double dist_hyp = sqrt(pow(dist_x,2.0) + pow(dist_y,2.0));

			time_t delta_t = this_fix.GetUnixTimestamp() - prev_fix.GetUnixTimestamp();
			ASSERT(delta_t > 0);

		//	TRACE("Distance moved = %f m.  Time taken = %d secs.\n", dist_hyp, delta_t);

			// Work that out in km/h and mph
			double speed_ms = (1000 * dist_hyp) / delta_t;
			double speed_kmh = 3.6 * speed_ms;
			double speed_mph = 5.0 * speed_kmh / 8.0;

			// Stash the speed in the position fix.
			i->SetSpeed(speed_ms);

//			TRACE("%d: lat_deg = %f, lon_deg = %f, dX = %f, dY = %f, d = %f, dT = %d, speed_kmh = %f, speed_mph = %f, speed_ms = %f\n",
//					row, this_fix.GetLatitude(), this_fix.GetLongitude(), dist_x, dist_y, dist_hyp, delta_t, speed_kmh, speed_mph, speed_ms);
			++row;

			if ((row % 800) == 0)
				TRACE(".\n");

			prev_fix = this_fix;
		}
	}

	// Calculate acceleration
	if (m_positions.size() >= 3)
	{
		// We'll do this in 3 passes (and optimise it later).
		PositionFixCollection::iterator b = m_positions.begin();
		++b;	// The first one doesn't have a speed in it.
		PositionFix prev_fix = *b++;

		PositionFixCollection::iterator e = m_positions.end();

		int row = 4;
		for (PositionFixCollection::iterator i = b; i != e; ++i)
		{
			PositionFix this_fix = *i;

			time_t delta_t = this_fix.GetUnixTimestamp() - prev_fix.GetUnixTimestamp();
			ASSERT(delta_t > 0);

			double prev_speed_ms = prev_fix.GetSpeed();
			double this_speed_ms = this_fix.GetSpeed();

			double this_acceleration = (this_speed_ms - prev_speed_ms) / delta_t;

			// Stash the speed in the position fix.
			i->SetAcceleration(this_acceleration);

			const double NEG_ACCELERATION_THRESHOLD = -3.5;
			const double POS_ACCELERATION_THRESHOLD = 3.5;
			if (this_acceleration > NEG_ACCELERATION_THRESHOLD && this_acceleration < POS_ACCELERATION_THRESHOLD)
			{
				// It's probably kosher
			}
			else
				i->SetBogus();

//			TRACE("%d: lat_deg = %f, lon_deg = %f, dT = %d, speed_ms = %f, accel = %f, bogus = %c\n",
//				row, this_fix.GetLatitude(), this_fix.GetLongitude(), delta_t, this_speed_ms, this_acceleration, i->IsBogus() ? 'Y' : 'N');
			++row;

			if ((row % 800) == 0)
				TRACE(".\n");

			prev_fix = this_fix;
		}
	}

	// Another pass: filter for actual bogosity.
	const int ARBITRARY_LOOK_AHEAD = 5;
	if (m_positions.size() >= 3 + ARBITRARY_LOOK_AHEAD)
	{
		PositionFixCollection::iterator b = m_positions.begin() + 2;
		PositionFixCollection::iterator e = m_positions.end() - ARBITRARY_LOOK_AHEAD;

		for (PositionFixCollection::iterator i = b; i != e; ++i)
		{
			if (i->IsBogus())
			{
				// Check the next few points.  If they're _all_ not bogus, then this one isn't.
				// We can start from the next one, which'll make this loop simpler.
				int bogusCount = 0;
				for (int n = 0; n < ARBITRARY_LOOK_AHEAD; ++n)
				{
					PositionFixCollection::iterator j = i + n;

					if (j->IsBogus())
						++bogusCount;
				}

				ASSERT(bogusCount > 0);

				// The only bogus point was this one.
				if (bogusCount == 1)
					i->SetBogus(false);
			}
		}
	}
#endif

	RecalcRoute();

	if (pProgress)
		pProgress->OnComplete();
}

bool CSnoTrackDoc::FindUploadWizard(CString *pUploadWizard)
{
	// At the moment, we'll assume that the file uploading wizard is in the same directory.
	CString moduleFilename;
	GetModuleFileName(NULL, moduleFilename.GetBuffer(MAX_PATH), MAX_PATH);
	moduleFilename.ReleaseBuffer();

	int pos = moduleFilename.ReverseFind('\\');
	if (pos == -1)
	{
		AfxMessageBox(IDS_FAILED_TO_COMPOSE_PATH_TO_UPLOAD_WIZARD, MB_ICONEXCLAMATION);
		return false;
	}

#ifdef _DEBUG
	CString installationDirectory = moduleFilename.Left(pos);
	pos = installationDirectory.ReverseFind('\\');
	CString pathComponent = installationDirectory.Mid(pos+1);
	CString uploadWizard;
	if (pathComponent.CompareNoCase(_T("Debug")) == 0)
	{
		uploadWizard = installationDirectory;
		uploadWizard.Replace(_T("SnoTrack"), _T("PostFile"));
		uploadWizard.Append(_T("\\PostFile.exe"));
	}
#else
	// Leave the trailing backslash on there.
	CString installationDirectory = moduleFilename.Left(pos+1);

	CString uploadWizard = installationDirectory + _T("PostFile.exe");
#endif
	CFileStatus status;
	if (!CFile::GetStatus(uploadWizard, status) ||
		(status.m_attribute & CFile::directory))
	{
		AfxMessageBox(IDS_FAILED_TO_FIND_UPLOAD_WIZARD, MB_ICONEXCLAMATION);
		return false;
	}

	*pUploadWizard = uploadWizard;
	return true;
}

void CSnoTrackDoc::OnFileUploadfile()
{
	if (!SaveModified())
		return;

	CString uploadWizard;
	if (!FindUploadWizard(&uploadWizard))
		return;

	// Make sure that the pathname is quoted.
	CString parameters;
	parameters.Format(_T("\"%s\""), GetPathName());
	INT_PTR result = (INT_PTR)ShellExecute(NULL, _T("open"), uploadWizard, parameters, NULL, SW_SHOWNORMAL);
	if (result <= 32)
	{
		// Then it failed: prompt the user.
		CString error = FormatShellExecuteErrorMessage(result, uploadWizard);
		AfxMessageBox(error, MB_ICONEXCLAMATION);
		return;
	}
}

CString CSnoTrackDoc::GetDefaultFileExtension()
{
	CString strExt;
	if (m_pDocTemplate->GetDocString(strExt, CDocTemplate::filterExt) && !strExt.IsEmpty())
	{
		ASSERT(strExt[0] == '.');
		int iStart = 0;
		CString defaultExtension = strExt.Tokenize(_T(";"), iStart);

		return defaultExtension;
	}

	return CString();
}

CString CSnoTrackDoc::BuildExportFileName(CString exportParen, CString exportExtension)
{
	CString defaultExtension = GetDefaultFileExtension();

	CString strFileName = GetPathName();
	if (strFileName.IsEmpty())
	{
		strFileName = m_strTitle;

		// check for dubious filename
		int iBad = strFileName.FindOneOf(_T(":/\\"));
		if (iBad != -1)
			strFileName.ReleaseBuffer(iBad);

		strFileName += defaultExtension;
	}

	if (strFileName.Right(defaultExtension.GetLength()) == defaultExtension)
	{
		strFileName = strFileName.Left(strFileName.GetLength() - defaultExtension.GetLength());
		if (!exportParen.IsEmpty())
			strFileName += _T(" ") + exportParen;
		if (!exportExtension.IsEmpty())
			strFileName += _T(".") + exportExtension;
	}

	return strFileName;
}

void CSnoTrackDoc::OnExportTracklogToFile()
{
	CString strFileName = BuildExportFileName(_T("(Track Log)"), _T("csv"));

	CFileDialog dlg(FALSE, _T("csv"), strFileName, OFN_OVERWRITEPROMPT, _T("Comma Separated Values (*.csv)|*.csv||"));
	INT_PTR nResult = dlg.DoModal();
	if (nResult != IDOK)
		return;

	CWaitCursor wait;

	// Open the file and write out the header:
	const char tracklogHeader[] = "TRK_IDENT,LAT,LON,TIME,ALT,DPTH,NEW,LAT_DEG,LON_DEG,TIME_GMT\n";
	FILE *fp = _tfopen(dlg.GetPathName(), _T("w"));
	if (!fp)
	{
		CString message;
		message.Format(IDS_FAILED_OPEN_EXPORT_TRACKLOG_CSV, errno);
		AfxMessageBox(message);
		return;
	}

	fputs(tracklogHeader, fp);

	// Write out the rest of the data:
	for (PositionFixCollection::const_iterator i = m_positions.begin(); i != m_positions.end(); ++i)
	{
		std::string trackIdent = i->GetTrackIdent();
		long latInSemicircles = SemicirclesFromDegrees((float)i->GetLatitude());
		long lonInSemicircles = SemicirclesFromDegrees((float)i->GetLongitude());
		long garminTimestamp = GarminTimeFromFileTime(i->GetTimestamp());
		double altitude = i->GetGpsElevation();
		double depth = 0.0;
		bool newTrack = i->IsNewTrack();
		double latInDegrees = i->GetLatitude();
		double lonInDegrees = i->GetLongitude();

		std::string gmtTimestamp = FormatGarminTime(garminTimestamp);

		CString tracklogLine;
		tracklogLine.Format("%s,%d,%d,%d,%f,%f,%c,%f,%f,%s\n",
			trackIdent.c_str(), latInSemicircles, lonInSemicircles,
			garminTimestamp, altitude, depth,
			newTrack ? 'Y' : 'N', latInDegrees, lonInDegrees,
			gmtTimestamp.c_str());

		fputs(tracklogLine, fp);
	}

	fclose(fp);
}

void CSnoTrackDoc::OnExportWaypointsToFile()
{
	CString strFileName = BuildExportFileName(_T("(Waypoints)"), _T("csv"));

	CFileDialog dlg(FALSE, _T("csv"), strFileName, OFN_OVERWRITEPROMPT, _T("Comma Separated Values (*.csv)|*.csv||"));
	INT_PTR nResult = dlg.DoModal();
	if (nResult != IDOK)
		return;

	CWaitCursor wait;

	// Open the file and write out the header:
	const char tracklogHeader[] = "LAT,LON,LAT_DEG,LON_DEG,IDENT\n";
	FILE *fp = _tfopen(dlg.GetPathName(), "w");
	if (!fp)
	{
		//"Failed to open file '%s', errno = %d", errno);
		CString message;
		message.Format(IDS_FAILED_OPEN_EXPORT_WAYPOINT_CSV, errno);
		AfxMessageBox(message);
		return;
	}

	fputs(tracklogHeader, fp);

	// Write out the rest of the data:
	for (WaypointCollection::const_iterator i = m_user_waypoints.begin(); i != m_user_waypoints.end(); ++i)
	{
		long latInSemicircles = SemicirclesFromDegrees((float)i->GetLatitude());
		long lonInSemicircles = SemicirclesFromDegrees((float)i->GetLongitude());
		double latInDegrees = i->GetLatitude();
		double lonInDegrees = i->GetLongitude();
		std::string label = i->GetLabel();

		CString waypointLine;
		waypointLine.Format("%d,%d,%f,%f,%s\n",
			latInSemicircles, lonInSemicircles,
			latInDegrees, lonInDegrees,
			label.c_str());

		fputs(waypointLine, fp);
	}

	fclose(fp);
}

BOOL CSnoTrackDoc::OnSaveDocument(LPCTSTR lpszPathName)
{
	CFileException fe;
	CFile* pFile = NULL;
	pFile = GetFile(lpszPathName, CFile::modeCreate |
		CFile::modeReadWrite | CFile::shareExclusive, &fe);

	if (pFile == NULL)
	{
		ReportSaveLoadException(lpszPathName, &fe,
			TRUE, AFX_IDP_INVALID_FILENAME);
		return FALSE;
	}

	TRY
	{
		CWaitCursor wait;

		OnSaveDocument(pFile);

		ReleaseFile(pFile, FALSE);
	}
	CATCH_ALL(e)
	{
		ReleaseFile(pFile, TRUE);

		TRY
		{
			ReportSaveLoadException(lpszPathName, e,
				TRUE, AFX_IDP_FAILED_TO_SAVE_DOC);
		}
		END_TRY
		return FALSE;
	}
	END_CATCH_ALL

	SetModifiedFlag(FALSE);     // back to unmodified

	return TRUE;        // success
}

BOOL CSnoTrackDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
#ifdef _DEBUG
	if (IsModified())
		ATLTRACE(traceAppMsg, 0, "Warning: OnOpenDocument replaces an unsaved document.\n");
#endif

	CFileException fe;
	CFile* pFile = GetFile(lpszPathName,
		CFile::modeRead|CFile::shareDenyWrite, &fe);
	if (pFile == NULL)
	{
		ReportSaveLoadException(lpszPathName, &fe,
			FALSE, AFX_IDP_FAILED_TO_OPEN_DOC);
		return FALSE;
	}

	DeleteContents();
	SetModifiedFlag();  // dirty during de-serialize

	TRY
	{
		CWaitCursor wait;
		if (pFile->GetLength() != 0)
		{
			OnOpenDocument(pFile);
		}

		ReleaseFile(pFile, FALSE);
	}
	CATCH_ALL(e)
	{
		ReleaseFile(pFile, TRUE);
		DeleteContents();   // remove failed contents

		TRY
		{
			ReportSaveLoadException(lpszPathName, e,
				FALSE, AFX_IDP_FAILED_TO_OPEN_DOC);
		}
		END_TRY
		return FALSE;
	}
	END_CATCH_ALL

	SetModifiedFlag(FALSE);     // start off with unmodified

	return TRUE;
}

BOOL CSnoTrackDoc::OnOpenDocument(CFile *pFile)
{
	CSnoTrackFileReader reader;
	reader.ReadFile(pFile, this);

	ProcessWaypointsAndTrackLog(NULL);
	return TRUE;
}

BOOL CSnoTrackDoc::OnSaveDocument(CFile *pFile)
{
	CSnoTrackFileWriter writer;
	writer.WriteFile(pFile, this);
	return TRUE;
}

// CSnoTrackFileWriterCallback
void CSnoTrackDoc::GetProductData(Product_Data_Type *productData)
{
	*productData = m_productData;
}

tstring CSnoTrackDoc::GetResortFileName()
{
	return m_resortFileName;
}

utf8_string CSnoTrackDoc::GetCustomResortName()
{
	return m_customResortName;
}

utf8_string CSnoTrackDoc::GetComments()
{
	return m_comments;
}

DWORD CSnoTrackDoc::GetTracklogSize()
{
	return (DWORD)m_positions.size();
}

void CSnoTrackDoc::GetTracklog(TracklogWriterCallback *pCallback)
{
	for (PositionFixCollection::const_iterator i = m_positions.begin(); i != m_positions.end(); ++i)
	{
		pCallback->WritePositionFix(i->GetTimestamp(), i->GetTrackIdent().c_str(), i->IsNewTrack(),
									i->GetLongitude(), i->GetLatitude(), i->GetGpsElevation());
	}
}

DWORD CSnoTrackDoc::GetWaypointCount()
{
	return (DWORD)m_user_waypoints.size();
}

void CSnoTrackDoc::GetWaypoints(WaypointWriterCallback *pCallback)
{
	DWORD dwWaypointCount = (DWORD)m_user_waypoints.size();

	for (WaypointCollection::const_iterator i = m_user_waypoints.begin(); i != m_user_waypoints.end(); ++i)
	{
		pCallback->WriteWaypoint(i->GetLabel().c_str(), i->GetLongitude(), i->GetLatitude());
	}
}

// CSnoTrackFileReaderCallback
void CSnoTrackDoc::SetProductData(const Product_Data_Type &productData)
{
	m_productData = productData;
}

void CSnoTrackDoc::SetComments(const utf8_string &comments)
{
	m_comments = comments;
}

void CSnoTrackDoc::SetResortFileName(const tstring &resortFileName)
{
	if (!resortFileName.empty())
		OnOpenResort(resortFileName.c_str());
	else
		ClearResortDetails();
}

void CSnoTrackDoc::SetCustomResortName(const utf8_string &customResortName)
{
	m_customResortName = customResortName;
}

void CSnoTrackDoc::SetTracklogSize(DWORD dwTracklogSize)
{
	m_positions.clear();
	m_positions.reserve(dwTracklogSize);
}

void CSnoTrackDoc::AddPositionFix(FILETIME timestamp, const utf8_string &trk_ident, bool is_new, double lon_deg, double lat_deg, double gps_elev)
{
	CPoint p = m_proj.ForwardDeg(lon_deg, lat_deg);
	PositionFix position(trk_ident, lon_deg, lat_deg, timestamp, gps_elev, is_new, p.x, p.y);
	m_positions.push_back(position);
}

void CSnoTrackDoc::SetWaypointCount(DWORD dwWaypointCount)
{
	m_user_waypoints.clear();
	m_user_waypoints.reserve(dwWaypointCount);
}

void CSnoTrackDoc::AddWaypoint(const utf8_string &label, double lon_deg, double lat_deg)
{
	CPoint p = m_proj.ForwardDeg(lon_deg, lat_deg);
	Waypoint waypoint(label, lon_deg, lat_deg, p.x, p.y);
	m_user_waypoints.push_back(waypoint);
}

void CSnoTrackDoc::OnImportNmeafromfile()
{
	CString nmeaFile;
	if (!DoOpenFileDialog(_T("Directories"), _T("NMEA Files"), NULL, NULL, &nmeaFile))
		return;

	CImportFileProgressDialog progress;
	progress.Create();

	NmeaSentenceCollector sentenceCollector(&m_positions, &m_proj);
	ImportPositionsFromFile(nmeaFile, &sentenceCollector, &progress);
	ProcessWaypointsAndTrackLog(&progress);

	progress.DestroyWindow();

	SetModifiedFlag(TRUE);
	UpdateAllViews(NULL, HINT_IMPORT_TRACKLOG);
}

void CSnoTrackDoc::OnImportFromSnologFile()
{
	CString snologFile;
	if (!DoOpenFileDialog(_T("Directories"), _T("snolog Files"), NULL, NULL, &snologFile))
		return;

	CImportFileProgressDialog progress;
	progress.Create();

	utf8_string trk_ident = "SNOLOG";
	SnologSentenceCollector sentenceCollector(trk_ident, &m_positions, &m_proj);
	ImportPositionsFromFile(snologFile, &sentenceCollector, &progress);
	ProcessWaypointsAndTrackLog(&progress);

	progress.DestroyWindow();

	SetModifiedFlag(TRUE);
	UpdateAllViews(NULL, HINT_IMPORT_TRACKLOG);
}

bool CSnoTrackDoc::IsEmptyDataSet() const
{
	return (m_positions.empty() && m_user_waypoints.empty());
}

BOOL CSnoTrackDoc::OnOpenResort(LPCTSTR lpszPathName)
{
	CFileException fe;
	CFile* pFile = GetFile(lpszPathName,
		CFile::modeRead|CFile::shareDenyWrite, &fe);
	if (pFile == NULL)
	{
		ReportSaveLoadException(lpszPathName, &fe,
			FALSE, AFX_IDP_FAILED_TO_OPEN_DOC);
		return FALSE;
	}

	DeleteContents();
	SetModifiedFlag();  // dirty during de-serialize

	TRY
	{
		CWaitCursor wait;
		if (pFile->GetLength() != 0)
		{
			OnOpenResort(pFile);
		}

		ReleaseFile(pFile, FALSE);
	}
	CATCH_ALL(e)
	{
		ReleaseFile(pFile, TRUE);
		DeleteContents();   // remove failed contents

		TRY
		{
			ReportSaveLoadException(lpszPathName, e,
				FALSE, AFX_IDP_FAILED_TO_OPEN_DOC);
		}
		END_TRY
		return FALSE;
	}
	END_CATCH_ALL

//	SetModifiedFlag(FALSE);     // start off with unmodified
	m_resortFileName = lpszPathName;

	return TRUE;
}

static unsigned char resort_magic_v0_0_50[] = { 0x10, 0xc3, 0x0e, 0xc0, 0xaf, 0xbb, 0x40, 0x77, };
static unsigned char resort_magic_v0_0_56[] = { 0x43, 0xa8, 0xfb, 0xda, 0x1a, 0xd1, 0xdc, 0xd6, };
static unsigned char resort_magic_v0_0_65[] = { 0xc6, 0xc3, 0xa1, 0x9c, 0x60, 0x18, 0xdc, 0x5a, };
static unsigned char resort_magic_v0_0_70[] = { 0xd7, 0xeb, 0xfd, 0xfc, 0xb0, 0x71, 0x19, 0x8d, };

void CSnoTrackDoc::OnOpenResort(CFile *pFile)
{
	CFileHelper helper(pFile);

	ASSERT(sizeof(resort_magic_v0_0_50) == 8);
	ASSERT(sizeof(resort_magic_v0_0_56) == 8);
	ASSERT(sizeof(resort_magic_v0_0_65) == 8);
	ASSERT(sizeof(resort_magic_v0_0_70) == 8);

	unsigned char magic[8];
	helper.ReadMagic(magic);

	int fileFormat = 0;
	if (memcmp(magic, resort_magic_v0_0_50, 8) == 0)
		fileFormat = 50;
	else if (memcmp(magic, resort_magic_v0_0_56, 8) == 0)
		fileFormat = 56;
	else if (memcmp(magic, resort_magic_v0_0_65, 8) == 0)
		fileFormat = 65;
	else if (memcmp(magic, resort_magic_v0_0_70, 8) == 0)
		fileFormat = 70;
	else
		AfxThrowArchiveException(CArchiveException::badIndex, pFile->GetFileName());

	if (fileFormat >= 50)
	{
		// Then we need to write the resort's name, country, timezone and orientation:
		CFileHelper helper(pFile);
		m_resortName = helper.ReadString2();
		m_resortTimeZone = helper.ReadString2();
		m_dwResortOrientation = helper.ReadInteger();

		// Then come the resort waypoints:
		m_resort_waypoints.clear();
		DWORD waypointCount = helper.ReadInteger();
		m_resort_waypoints.reserve(waypointCount);
		for (DWORD i = 0; i < waypointCount; ++i)
		{
			utf8_string label = helper.ReadString1();
			double lon = helper.ReadDouble();
			double lat = helper.ReadDouble();

			CPoint p = m_proj.ForwardDeg(lon, lat);
			m_resort_waypoints.push_back(Waypoint(label, lon, lat, p.x, p.y));
		}

		// Immediately following that is the list of resort lifts:
		m_lifts.clear();
		DWORD liftCount = helper.ReadInteger();
		m_lifts.reserve(liftCount);
		for (DWORD i = 0; i < liftCount; ++i)
		{
			utf8_string label = helper.ReadString1();
			double bot_lon = helper.ReadDouble();
			double bot_lat = helper.ReadDouble();
			double top_lon = helper.ReadDouble();
			double top_lat = helper.ReadDouble();

			CPoint bot = m_proj.ForwardDeg(bot_lon, bot_lat);
			CPoint top = m_proj.ForwardDeg(top_lon, top_lat);

			m_lifts.push_back(Lift(label, bot_lon, bot_lat, bot.x, bot.y, top_lon, top_lat, top.x, top.y));
		}

		delete m_hotspotGrid;
		bool hasHotspotGrid = helper.ReadBool();
		if (hasHotspotGrid)
		{
			m_hotspotGrid = NEW HotspotGrid;

			DWORD hotspotCount = helper.ReadInteger();
			DWORD cellSpacing = helper.ReadInteger();
			CRect gridRange;
			gridRange.left = helper.ReadInteger();
			gridRange.top = helper.ReadInteger();
			gridRange.right = helper.ReadInteger();
			gridRange.bottom = helper.ReadInteger();

			m_hotspotGrid->Create(gridRange, cellSpacing);

			for (DWORD i = 0; i < hotspotCount; ++i)
			{
				DWORD xCell = helper.ReadInteger();
				DWORD yCell = helper.ReadInteger();
				DWORD id = helper.ReadInteger();

				m_hotspotGrid->AddHotspot(xCell, yCell, id);
			}
		}

		// And now save the hotspot paths:
		DWORD pathCount = helper.ReadInteger();
		m_hotspotPaths.clear();
		m_hotspotPaths.reserve(pathCount);
		for (DWORD i = 0; i < pathCount; ++i)
		{
			utf8_string name = helper.ReadString2();

			bool isLift = false;
			if (fileFormat >= 65)
				isLift = helper.ReadBool();

			int imageIndex = 0;
			if (fileFormat >= 56)
				imageIndex = helper.ReadInteger();

			DWORD spotCount = helper.ReadInteger();

			HotspotPath path(name, isLift, imageIndex);
			path.reserve(spotCount);

			for (DWORD j = 0; j < spotCount; ++j)
			{
				DWORD id = helper.ReadInteger();
				path.push_back(id);
			}

			m_hotspotPaths.push_back(path);
		}
	}

	// Followed by the hotspot names:
	if (fileFormat >= 70)
	{
		DWORD nameCount = helper.ReadInteger();
		m_hotspotNames.clear();

		for (DWORD i = 0; i < nameCount; ++i)
		{
			int hotspotID = helper.ReadInteger();
			utf8_string name = helper.ReadString2();

			m_hotspotNames[hotspotID] = name;
		}
	}
}

void CSnoTrackDoc::ClearResortDetails()
{
	m_resortName.clear();
	m_resortTimeZone.clear();
	m_dwResortOrientation = 0;

	m_resort_waypoints.clear();
	m_lifts.clear();
	delete m_hotspotGrid;
	m_hotspotPaths.clear();
}

class CentreOfGravityCallback : public HotspotGridCallback
{
	centres_t *m_centres;

public:
	CentreOfGravityCallback(centres_t *centres)
		: m_centres(centres)
	{
		m_centres->clear();
	}

// HotspotGridCallback
public:
	virtual void OnHotspot(int xCell, int yCell, int hotspotId, CRect logicalRect)
	{
		// We need to work out the c.o.g. of the hotspot.
		// We can keep a running average for that hotspot.
		CPoint thisCentreOfGravity = logicalRect.CenterPoint();
		CentreOfGravity oldCentreOfGravity = (*m_centres)[hotspotId];

		// If we're moving a centre-of-gravity, then it's ((w1 * p1) + (w2 * p2)) / w1 + w2
		CPoint p;
		p.x = (LONG)((Int32x32To64(oldCentreOfGravity.point.x, oldCentreOfGravity.weight)
			+ Int32x32To64(thisCentreOfGravity.x, 1)) / (oldCentreOfGravity.weight + 1));
		p.y = (LONG)((Int32x32To64(oldCentreOfGravity.point.y, oldCentreOfGravity.weight)
			+ Int32x32To64(thisCentreOfGravity.y, 1)) / (oldCentreOfGravity.weight + 1));

		CentreOfGravity newCentreOfGravity(p, oldCentreOfGravity.weight+1);
		(*m_centres)[hotspotId] = newCentreOfGravity;
	}
};

double CalculateCircleDistance(double radlon1, double radlat1, double radlon2, double radlat2)
{
	double er = 6366.707;

	double x1 = er * cos(radlon1)*sin(radlat1);
	double y1 = er * sin(radlon1)*sin(radlat1);
	double z1 = er * cos(radlat1);

	double x2 = er * cos(radlon2)*sin(radlat2);
	double y2 = er * sin(radlon2)*sin(radlat2);
	double z2 = er * cos(radlat2);

	double d = sqrt((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2)+(z1-z2)*(z1-z2));

	//side, side, side, law of cosines and arccos
	double theta = acos((er*er+er*er-d*d)/(2*er*er));
	double distance = theta*er;

	return distance;
}

double CalculateCircleDistance(const Projection &projection, CPoint p1, CPoint p2)
{
	projUV uv1 = projection.ReverseRad(p1);
	projUV uv2 = projection.ReverseRad(p2);

	double londeg1 = RadiansToDegrees(uv1.u);
	double latdeg1 = RadiansToDegrees(uv1.v);

	double londeg2 = RadiansToDegrees(uv2.u);
	double latdeg2 = RadiansToDegrees(uv2.v);

//	TRACE("Point 1: %s; Point 2: %s", LPCTSTR(FormatLatLonPosition(latdeg1, londeg1)), LPCTSTR(FormatLatLonPosition(latdeg2, londeg2)));

	return CalculateCircleDistance(uv1.u, uv1.v, uv2.u, uv2.v);
}

double CalculateHotspotDistance(const Projection &projection, centres_t centres, int prev_id, int this_id)
{
	CentreOfGravity c1 = centres[prev_id];
	CentreOfGravity c2 = centres[this_id];

	CPoint p1 = c1.point;
	CPoint p2 = c2.point;

	// p1 and p2 are in logical coordinates (i.e. 10cm units).
	double hypot_distance_km = hypot(p1.y - p2.y, p1.x - p2.x) / 10000;

	// Also work it out (for checking) using the lat/lon of the ends.
	double circle_distance_km = CalculateCircleDistance(projection, p1, p2);

//	TRACE("hypot_distance_km = %f, circle_distance_km = %f\n", hypot_distance_km, circle_distance_km);

	return circle_distance_km;
}

double CalculatePathDistance(const Projection &projection, centres_t centres, const HotspotPath &path)
{
	ASSERT(path.size() >= 2);

	double distance = 0.0;

	HotspotPath::const_iterator b = path.begin();
	HotspotPath::const_iterator e = path.end();
	for (HotspotPath::const_iterator i = b + 1; i != e; ++i)
	{
		int prev_id = *(i-1);
		int this_id = *i;

		distance += CalculateHotspotDistance(projection, centres, prev_id, this_id);
	}

	return distance;
}

void CSnoTrackDoc::RecalcRoute()
{
	if (!m_hotspotGrid)
		return;

#ifdef _DEBUG
	double distance_london_ny = CalculateCircleDistance(DegreesToRadians(-0.166666), DegreesToRadians(51.5), DegreesToRadians(-73.99416666), DegreesToRadians(40.7516666));
#endif

	// Recalculate the centrepoints of the hotspots:
	CentreOfGravityCallback centreCallback(&m_centres);
	m_hotspotGrid->EnumerateHotspots(&centreCallback);

//	for (centres_t::const_iterator i = m_centres.begin(); i != m_centres.end(); ++i)
//	{
//		TRACE("Hotspot #%d: x = %d, y = %d (w = %d)\n", i->first, i->second.point.x, i->second.point.y, i->second.weight);
//	}

	// Iterate over the paths, filling in the distances.
	HotspotPathCollection::iterator b = m_hotspotPaths.begin();
	HotspotPathCollection::iterator e = m_hotspotPaths.end();
	for (HotspotPathCollection::iterator i = b; i != e; ++i)
	{
		double distance = 0;
		if (i->size() >= 2)
		{
			distance = CalculatePathDistance(m_proj, m_centres, *i);
		}

//		TRACE("Path '%s' is %f m.\n", ConvertUTF8ToSystem(i->GetName()).c_str(), distance);
		i->SetDistance(distance);
	}

	// Recalculate the route:
	HotspotVisitCollection visitedHotspots;
	CalculateHotspotVisits(m_positions, m_hotspotGrid, &visitedHotspots);

	m_route.clear();
	RouteEventCollector routeCollector(&m_route);
	CalculateRoute(m_hotspotPaths, m_hotspotNames, visitedHotspots, &routeCollector);
}

#if defined(ENABLE_DESIGN_MODE)
void CSnoTrackDoc::OnResortNew()
{
	if (!SaveModifiedResort())
		return;

	ClearResortDetails();

	RecalcRoute();
	UpdateAllViews(NULL, HINT_ALL_RESORT);
}

void CSnoTrackDoc::OnResortOpen()
{
	if (!SaveModifiedResort())
		return;

	CString resortFile;
	if (!DoOpenFileDialog(_T("Directories"), _T("Resort Files"), _T("resort"), _T("Resort Files (*.resort)|*.resort||"), &resortFile))
		return;

	OnOpenResort(resortFile);
	RecalcRoute();
	UpdateAllViews(NULL, HINT_ALL_RESORT);
}

void CSnoTrackDoc::OnResortSave()
{
	OnSaveResort(m_resortFileName.c_str());
}

void CSnoTrackDoc::OnResortSaveas()
{
	CFileDialog dlg(FALSE, _T("resort"), m_resortFileName.c_str(), OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, _T("Resort Files (*.resort)|*.resort||"));
	INT_PTR nResult = dlg.DoModal();
	if (nResult == IDOK)
	{
		m_resortFileName = dlg.GetPathName();
		OnSaveResort(m_resortFileName.c_str());
	}
}

void CSnoTrackDoc::OnResortEditLifts()
{
	CEditLiftsDialog dlg(m_lifts, &m_proj);
	if (dlg.DoModal() == IDOK)
		dlg.GetLifts(&m_lifts);

	SetResortModified(true);
	UpdateAllViews(NULL, HINT_EDIT_RESORT_LIFTS);
}

void CSnoTrackDoc::OnResortEditWaypoints()
{
	CEditWaypointsDialog dlg(m_resort_waypoints, &m_proj);
	if (dlg.DoModal() == IDOK)
		dlg.GetWaypoints(&m_resort_waypoints);

	SetResortModified(true);
	UpdateAllViews(NULL, HINT_EDIT_RESORT_WAYPOINTS);
}

BOOL CSnoTrackDoc::OnSaveResort(LPCTSTR lpszPathName)
{
	CFileException fe;
	CFile* pFile = NULL;
	pFile = GetFile(lpszPathName, CFile::modeCreate |
		CFile::modeReadWrite | CFile::shareExclusive, &fe);

	if (pFile == NULL)
	{
		ReportSaveLoadException(lpszPathName, &fe,
			TRUE, AFX_IDP_INVALID_FILENAME);
		return FALSE;
	}

	TRY
	{
		CWaitCursor wait;

		OnSaveResort(pFile);

		ReleaseFile(pFile, FALSE);
	}
	CATCH_ALL(e)
	{
		ReleaseFile(pFile, TRUE);

		TRY
		{
			ReportSaveLoadException(lpszPathName, e,
				TRUE, AFX_IDP_FAILED_TO_SAVE_DOC);
		}
		END_TRY
		return FALSE;
	}
	END_CATCH_ALL

	SetResortModified(false);
	return TRUE;        // success
}

class SaveHotspots : public HotspotGridCallback
{
	CFileHelper *m_pHelper;

public:
	SaveHotspots(CFileHelper *pHelper)
		: m_pHelper(pHelper)
	{
	}

	virtual void OnHotspot(int xCell, int yCell, int hotspotId, CRect logicalRect)
	{
		m_pHelper->WriteInteger(xCell);
		m_pHelper->WriteInteger(yCell);
		m_pHelper->WriteInteger(hotspotId);
	}
};

void CSnoTrackDoc::OnSaveResort(CFile *pFile)
{
    CFileHelper helper(pFile);

	// OK, so the first thing we need to do is to write the header to the file:
	ASSERT(sizeof(resort_magic_v0_0_70) == 8);
	helper.WriteMagic(resort_magic_v0_0_70);

	// Then we need to write the resort's name, country, timezone and orientation:
	helper.WriteString2(m_resortName);
	helper.WriteString2(m_resortTimeZone);
	helper.WriteInteger(m_dwResortOrientation);

	// Then come the resort waypoints:
	helper.WriteInteger((DWORD)m_resort_waypoints.size());
	for (WaypointCollection::const_iterator i = m_resort_waypoints.begin(); i != m_resort_waypoints.end(); ++i)
	{
		helper.WriteString1(i->GetLabel());
		helper.WriteDouble(i->GetLongitude());
		helper.WriteDouble(i->GetLatitude());
	}

	// Immediately following that is the list of resort lifts:
	helper.WriteInteger((DWORD)m_lifts.size());
	for (LiftCollection::const_iterator i = m_lifts.begin(); i != m_lifts.end(); ++i)
	{
		helper.WriteString1(i->GetLabel());
		helper.WriteDouble(i->GetBottomLongitude());
		helper.WriteDouble(i->GetBottomLatitude());
		helper.WriteDouble(i->GetTopLongitude());
		helper.WriteDouble(i->GetTopLatitude());
	}

	// Now save the hotspot grid:
	if (m_hotspotGrid)
	{
		helper.WriteBool(true);
		helper.WriteInteger(m_hotspotGrid->GetHotspotCount());
		helper.WriteInteger(m_hotspotGrid->GetCellSpacing());
		CRect gridRange = m_hotspotGrid->GetRange();
		helper.WriteInteger(gridRange.left);
		helper.WriteInteger(gridRange.top);
		helper.WriteInteger(gridRange.right);
		helper.WriteInteger(gridRange.bottom);

		SaveHotspots cb(&helper);
		m_hotspotGrid->EnumerateHotspots(&cb);
	}
	else
		helper.WriteBool(false);

	// And now save the hotspot paths:
	helper.WriteInteger((DWORD)m_hotspotPaths.size());
	for (HotspotPathCollection::const_iterator i = m_hotspotPaths.begin(); i != m_hotspotPaths.end(); ++i)
	{
		helper.WriteString2(i->GetName());
		helper.WriteBool(i->IsLift());
		helper.WriteInteger(i->GetImageIndex());

		helper.WriteInteger((DWORD)i->size());
		for (HotspotPath::const_iterator j = i->begin(); j != i->end(); ++j)
		{
			helper.WriteInteger(*j);
		}
	}

	// Then the hotspot names:
	helper.WriteInteger((DWORD)m_hotspotNames.size());
	for (HotspotNames::const_iterator i = m_hotspotNames.begin(); i != m_hotspotNames.end(); ++i)
	{
		helper.WriteInteger(i->first);
		helper.WriteString2(i->second);
	}
}

CRect CSnoTrackDoc::GetLogicalRangeFromGridBounds(const GridBounds &gridBounds) const
{
	// We need to project the corners, and then that'll be fine.
	const Projection *proj = GetProjection();
	CPoint topLeft = proj->ForwardDeg(gridBounds.m_minLongitude, gridBounds.m_minLatitude);
	CPoint bottomRight = proj->ForwardDeg(gridBounds.m_maxLongitude, gridBounds.m_maxLatitude);

	return CRect(topLeft, bottomRight);
}

bool CSnoTrackDoc::HasHotspotGrid() const
{
	return (m_hotspotGrid != NULL);
}

bool CSnoTrackDoc::CreateHotspotGrid()
{
	ASSERT(m_hotspotGrid == NULL);
	if (m_hotspotGrid)
		return false;

	GridBounds initialGridBounds = GetBounds();
	
	int initialGridSpacing = 10;	// in metres
	CDefineHotspotGridDialog dlg(initialGridBounds, initialGridSpacing);
	if (dlg.DoModal() != IDOK)
		return false;

	GridBounds gridBounds = dlg.GetGridBounds();
	int gridSpacing = dlg.GetGridSpacing();
	CRect range = GetLogicalRangeFromGridBounds(gridBounds);

	// Logical units are in 10cm units, so there are 10 of them to the metre:
	int cellSize = gridSpacing * 10;

	m_hotspotGrid = NEW HotspotGrid;
	if (m_hotspotGrid == NULL || !m_hotspotGrid->Create(range, cellSize))
	{
		AfxMessageBox("Failed to create hotspot grid");
		delete m_hotspotGrid;
		m_hotspotGrid = NULL;

		return false;
	}

	SetResortModified(true);
	return true;
}

void CSnoTrackDoc::DeleteHotspotGrid()
{
	if (m_hotspotGrid != NULL)
	{
		delete m_hotspotGrid;
		m_hotspotGrid = NULL;

		m_currentHotspotID = 0;
	}
}

void CSnoTrackDoc::OnHotspotsWalk()
{
	ASSERT(m_hotspotGrid != NULL);

	CWaitCursor wait;

	TIME_ZONE_INFORMATION tzi;
	GetRegistryTimeZoneInformation(GetTimeZoneName().c_str(), &tzi);
	CWalkHotspotsDialog dlg(&tzi, m_positions, m_hotspotGrid);
	dlg.DoModal();
}

void CSnoTrackDoc::OnUpdateHotspotsWalk(CCmdUI *pCmdUI)
{
	BOOL bEnable = (m_hotspotGrid != NULL);
	pCmdUI->Enable(bEnable);
}

void CSnoTrackDoc::OnUpdateHotspotsClearhotspots(CCmdUI *pCmdUI)
{
	BOOL bEnable = (m_hotspotGrid != NULL);
	pCmdUI->Enable(bEnable);
}

void CSnoTrackDoc::OnHotspotsSelectid()
{
	int nextHotspotID = m_hotspotGrid->SuggestNextHotspotID();
	CSelectHotspotIdDialog dlg(m_currentHotspotID, nextHotspotID);

	INT_PTR nResult = dlg.DoModal();
	if (nResult == IDOK)
	{
		m_currentHotspotID = dlg.GetNextID();
	}
}

void CSnoTrackDoc::OnUpdateHotspotsSelectid(CCmdUI *pCmdUI)
{
	BOOL bEnable = (m_hotspotGrid != NULL);
	pCmdUI->Enable(bEnable);
}

void CSnoTrackDoc::OnHotspotsEditpaths()
{
	CEditPathsDialog dlg(m_hotspotPaths);
	if (dlg.DoModal() == IDOK)
	{
		m_hotspotPaths = dlg.GetHotspotPaths();
		SetResortModified(true);
		RecalcRoute();
		UpdateAllViews(NULL, HINT_EDIT_RESORT_PATHS, NULL);
	}
}

void CSnoTrackDoc::OnUpdateHotspotsEditpaths(CCmdUI *pCmdUI)
{
	BOOL bEnable = (m_hotspotGrid != NULL);
	pCmdUI->Enable(bEnable);
}

void CSnoTrackDoc::ToggleHotspot(CPoint logicalPoint, int size)
{
	m_hotspotGrid->ToggleHotspot(logicalPoint, m_currentHotspotID, size);
	SetResortModified(true);
}

void CSnoTrackDoc::OnResortRecalculateroutes()
{
	RecalcRoute();
	UpdateAllViews(NULL);
}

void CSnoTrackDoc::OnResortHotspotnames()
{
	ASSERT(m_hotspotGrid);

	CEditHotspotNamesDialog dlg(m_hotspotGrid, m_hotspotNames);
	if (dlg.DoModal() == IDOK)
	{
		m_hotspotNames = dlg.GetHotspotNames();
		SetResortModified(true);
		RecalcRoute();
		UpdateAllViews(NULL);
	}
}

void CSnoTrackDoc::OnUpdateResortHotspotnames(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_hotspotGrid != NULL);
}
#endif //  defined(ENABLE_DESIGN_MODE)

BOOL CSnoTrackDoc::SaveModifiedResort()
{
	if (!IsResortModified())
		return TRUE;

#if defined(ENABLE_DESIGN_MODE)
	int nResult = AfxMessageBox("Resort details are modified.  Save them?", MB_YESNOCANCEL);
	switch (nResult)
	{
		case IDCANCEL:
			return FALSE;	// don't continue.

		case IDYES:
			OnResortSave();
			break;

		case IDNO:
			break;	// don't bother saving the resort file.
	}
#endif

	return TRUE;
}

BOOL CSnoTrackDoc::SaveModified()
{
	if (!SaveModifiedResort())
		return FALSE;

	return __super::SaveModified();
}
