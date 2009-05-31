// SnoTrackDoc.h : interface of the CSnoTrackDoc class
//


#pragma once
#include "PositionFix.h"
#include "Waypoint.h"
#include "Lift.h"
#include "SnoTrackDLL/SnoTrackFileWriter.h"
#include "SnoTrackDLL/SnoTrackFileReader.h"
#include "garmin/garmin_packet_ids.h"
#include "Projection.h"
#include "GridBounds.h"
#include "HotspotGrid.h"
#include "HotspotPath.h"
#include "CalcRoute.h"
#include "CentreOfGravity.h"
#include "HotspotNames.h"

class LineCollectorCallback;
class Progress;

class CSnoTrackDoc : public CDocument, private CSnoTrackFileWriterCallback, private CSnoTrackFileReaderCallback
{
	Projection m_proj;

	utf8_string m_comments;

	FILETIME m_min_timestamp, m_max_timestamp;
	double m_min_elev, m_max_elev;

	GridBounds m_gridBounds;
	Product_Data_Type m_productData;	///< The _last_ unit used to import any data over serial.

	// Resort properties
	tstring m_resortFileName;
	utf8_string m_resortName;
	DWORD m_dwResortOrientation;
	tstring m_resortTimeZone;
	bool m_bResortModified;

	utf8_string m_customResortName;

public:	// for now.
	PositionFixCollection m_positions;		///< The user's tracklog.

	WaypointCollection m_user_waypoints;		///< User-defined waypoints.
	WaypointCollection m_resort_waypoints;	///< Resort-defined waypoints.

	LiftCollection m_lifts;					///< Resort-defined lifts.

	HotspotGrid *m_hotspotGrid;			///< defines interesting spots, like the top and bottom of a lift, a restaurant, etc.
	int m_currentHotspotID;

	HotspotPathCollection m_hotspotPaths;
	HotspotNames m_hotspotNames;

	centres_t m_centres;

protected: // create from serialization only
	CSnoTrackDoc();
	DECLARE_DYNCREATE(CSnoTrackDoc)

	virtual BOOL SaveModified();

public:
	const Projection *GetProjection() const { return &m_proj; }

	FILETIME GetMinimumTimestamp() const { return m_min_timestamp; }
	FILETIME GetMaximumTimestamp() const { return m_max_timestamp; }

	double GetMinimumElevation() const { return m_min_elev; }
	double GetMaximumElevation() const { return m_max_elev; }

	GridBounds GetBounds() const { return m_gridBounds; }

	tstring GetTimeZoneName() const;
	DWORD GetDefaultOrientation() const { return m_dwResortOrientation; }

	bool IsEmptyDataSet() const;

	void RecalcRoute();
	const RouteEventCollection &GetRoute() const { return m_route; }

private:
	RouteEventCollection m_route;

public:
	virtual BOOL OnNewDocument();
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	BOOL OnOpenDocument(CFile *pFile);
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	BOOL OnSaveDocument(CFile *pFile);

// Implementation
public:
	virtual ~CSnoTrackDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	afx_msg void OnFileProperties();
	afx_msg void OnFileUploadfile();

	afx_msg void OnImportSerial();
	afx_msg void OnImportTracklogFromFile();
	afx_msg void OnImportWaypointsFromFile();

	afx_msg void OnImportNmeafromfile();
	afx_msg void OnImportFromSnologFile();

	afx_msg void OnExportTracklogToFile();
	afx_msg void OnExportWaypointsToFile();

#if defined(ENABLE_DESIGN_MODE)
	afx_msg void OnHotspotsWalk();
	afx_msg void OnUpdateHotspotsWalk(CCmdUI *pCmdUI);
	afx_msg void OnUpdateHotspotsClearhotspots(CCmdUI *pCmdUI);
	afx_msg void OnHotspotsSelectid();
	afx_msg void OnUpdateHotspotsSelectid(CCmdUI *pCmdUI);
	afx_msg void OnHotspotsEditpaths();
	afx_msg void OnUpdateHotspotsEditpaths(CCmdUI *pCmdUI);

	afx_msg void OnResortNew();
	afx_msg void OnResortOpen();
	afx_msg void OnResortSaveas();
	afx_msg void OnResortSave();

	afx_msg void OnResortEditLifts();
	afx_msg void OnResortEditWaypoints();
	afx_msg void OnResortRecalculateroutes();
#endif

	DECLARE_MESSAGE_MAP()

private:
	BOOL OnSaveResort(LPCTSTR lpszPathName);
	void OnSaveResort(CFile *pFile);

	BOOL OnOpenResort(LPCTSTR lpszPathName);
	void OnOpenResort(CFile *pFile);

	void ClearResortDetails();

protected:
	bool IsResortModified() const { return m_bResortModified; }
	void SetResortModified(bool bModified) { m_bResortModified = bModified; }
	BOOL SaveModifiedResort();

public:
	bool HasHotspotGrid() const;
	bool CreateHotspotGrid();
	void DeleteHotspotGrid();
	void ToggleHotspot(CPoint logicalPoint, int size);

private:
	CRect GetLogicalRangeFromGridBounds(const GridBounds &gridBounds) const;

private:
	void ImportPositionsFromFile(const TCHAR *pathName, LineCollectorCallback *pCallback, Progress *pProgress);
	void ProcessWaypointsAndTrackLog(Progress *pProgress);

	bool FindUploadWizard(CString *pUploadWizard);
	CString GetDefaultFileExtension();
	CString BuildExportFileName(CString exportParen, CString exportExtension);

// CSnoTrackFileWriterCallback
private:
	virtual void GetProductData(Product_Data_Type *productData);

	virtual tstring GetResortFileName();
	virtual utf8_string GetCustomResortName();

	virtual utf8_string GetComments();

	virtual DWORD GetTracklogSize();
	virtual void GetTracklog(TracklogWriterCallback *pCallback);

	virtual DWORD GetWaypointCount();
	virtual void GetWaypoints(WaypointWriterCallback *pCallback);

// CSnoTrackFileReaderCallback
private:
	virtual void SetProductData(const Product_Data_Type &productData);

	virtual void SetComments(const utf8_string &comments);
	virtual void SetResortFileName(const tstring &resortFileName);

	virtual void SetCustomResortName(const utf8_string &customResortName);
	
	virtual void SetTracklogSize(DWORD dwTracklogSize);
	virtual void AddPositionFix(FILETIME timestamp, const utf8_string &trk_ident, bool is_new, double lon_deg, double lat_deg, double gps_elev);

	virtual void SetWaypointCount(DWORD dwWaypointCount);
	virtual void AddWaypoint(const utf8_string &label, double lon_deg, double lat_deg);
public:
	afx_msg void OnResortHotspotnames();
	afx_msg void OnUpdateResortHotspotnames(CCmdUI *pCmdUI);
};

#define HINT_IMPORT_TRACKLOG		0x0001
#define HINT_IMPORT_WAYPOINTS		0x0002
#define HINT_ALL_SNOTRACK			0x00FF

#define HINT_EDIT_RESORT_LIFTS		0x0100
#define HINT_EDIT_RESORT_WAYPOINTS	0x0200
#define HINT_EDIT_RESORT_PATHS		0x0400
#define HINT_EDIT_RESORT_HOTSPOTS	0x0800
#define HINT_ALL_RESORT				0xFF00
