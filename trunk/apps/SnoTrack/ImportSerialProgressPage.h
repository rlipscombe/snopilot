/* OtherPage.h
 *
 * Copyright (C) 2003 Roger Lipscombe
 *     http://www.differentpla.net/~roger/
 */

#pragma once
#include "resource.h"
#include "win32_ui/WizardPage.h"
#include "ImportSerialTask.h"
#include "garmin/garmin_packet_ids.h"

/////////////////////////////////////////////////////////////////////////////
// CImportSerialProgressPage dialog

// Settings are accumulated in here as the user goes through the wizard.
class ImportSerialSettings;
class ImportSerialResults;

// The actual work gets done on this thread.
class ImportSerialTaskThread;

class CImportSerialProgressPage : public CWizardPage, public ImportSerialTaskObserver
{
	const ImportSerialSettings *m_pSettings;
	ImportSerialResults *m_pResults;
	DWORD m_lastProgressTime;	///< Used to throttle progress message posting.
	Product_Data_Type m_product;///< Used to hold the product info, while the PostMessage is in transit.
	PositionFixCollection m_positions;	///< Used to hold the tracklog passed to OnTaskComplete, while the PostMessage is in transit.
	WaypointCollection m_waypoints;	///< Used to hold the waypoints passed to OnTaskComplete, while the PostMessage is in transit.

    DECLARE_DYNAMIC(CImportSerialProgressPage)

public:
    CImportSerialProgressPage(const ImportSerialSettings *pSettings, ImportSerialResults *pResults);
    ~CImportSerialProgressPage();

    //{{AFX_DATA(CImportSerialProgressPage)
    enum { IDD = IDD_IMPORT_SERIAL_PROGRESS_PAGE };
	CProgressCtrl m_progressCtrl;
    //}}AFX_DATA

    //{{AFX_VIRTUAL(CImportSerialProgressPage)
    public:
    virtual BOOL OnSetActive();
	virtual BOOL OnQueryCancel();
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

// ImportSerialTaskObserver
public:
	virtual void OnTaskBegin();

	virtual void OnTrackLogProgress(int current, int maximum);
	virtual void OnTrackLogComplete();

	virtual void OnWaypointProgress(int current, int maximum);
	virtual void OnWaypointComplete();

	virtual void OnTaskComplete(STATUS taskStatus, const Product_Data_Type *product, const PositionFixCollection *positions, const WaypointCollection *waypoints);

	virtual bool QueryCancelTask();

protected:
	//{{AFX_MSG(CImportSerialProgressPage)
    //}}AFX_MSG
	afx_msg LRESULT OnTaskBeginStub(WPARAM wParam, LPARAM lParam);

	afx_msg LRESULT OnTrackLogProgressStub(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnTrackLogCompleteStub(WPARAM wParam, LPARAM lParam);

	afx_msg LRESULT OnWaypointProgressStub(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnWaypointCompleteStub(WPARAM wParam, LPARAM lParam);

	afx_msg LRESULT OnTaskCompleteStub(WPARAM wParam, LPARAM lParam);

    DECLARE_MESSAGE_MAP()

private:
	ImportSerialTaskThread *m_taskThread;
	BOOL m_bCancel;
};

