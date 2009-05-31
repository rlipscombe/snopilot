/* ImportSerialProgressPage.cpp
 *
 * Copyright (C) 2003 Roger Lipscombe
 *     http://www.differentpla.net/~roger/
 * Copyright (C) 2004 Snopilot
 *     http://www.snopilot.com/
 */

#include "StdAfx.h"
#include "ImportSerialProgressPage.h"
#include "ImportSerialTaskThread.h"
#include "ImportSerialSettings.h"

IMPLEMENT_DYNAMIC(CImportSerialProgressPage, CWizardPage)

/////////////////////////////////////////////////////////////////////////////
// CImportSerialProgressPage property page

CImportSerialProgressPage::CImportSerialProgressPage(const ImportSerialSettings *pSettings, ImportSerialResults *pResults)
    : CWizardPage(CImportSerialProgressPage::IDD, IDS_IMPORT_SERIAL_WIZARD_CAPTION, IDS_IMPORT_SERIAL_PROGRESS_PAGE_TITLE, (UINT)0),
		m_taskThread(NULL), m_bCancel(FALSE), m_pSettings(pSettings), m_pResults(pResults), m_lastProgressTime(0)
{
    //{{AFX_DATA_INIT(CImportSerialProgressPage)
    //}}AFX_DATA_INIT
}

CImportSerialProgressPage::~CImportSerialProgressPage()
{
}

void CImportSerialProgressPage::DoDataExchange(CDataExchange* pDX)
{
	CWizardPage::DoDataExchange(pDX);

	//{{AFX_DATA_MAP(CImportSerialProgressPage)
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_PROGRESS, m_progressCtrl);
}

#define WM_TASK_BEGIN (WM_APP + 201)
#define WM_TRACKLOG_PROGRESS (WM_APP + 202)
#define WM_TRACKLOG_COMPLETE (WM_APP + 203)
#define WM_WAYPOINT_PROGRESS (WM_APP + 204)
#define WM_WAYPOINT_COMPLETE (WM_APP + 205)
#define WM_TASK_COMPLETE (WM_APP + 206)

BEGIN_MESSAGE_MAP(CImportSerialProgressPage, CWizardPage)
    //{{AFX_MSG_MAP(CImportSerialProgressPage)
	ON_MESSAGE(WM_TASK_BEGIN, OnTaskBeginStub)
	ON_MESSAGE(WM_TRACKLOG_PROGRESS, OnTrackLogProgressStub)
	ON_MESSAGE(WM_TRACKLOG_COMPLETE, OnTrackLogCompleteStub)
	ON_MESSAGE(WM_WAYPOINT_PROGRESS, OnWaypointProgressStub)
	ON_MESSAGE(WM_WAYPOINT_COMPLETE, OnWaypointCompleteStub)
	ON_MESSAGE(WM_TASK_COMPLETE, OnTaskCompleteStub)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CImportSerialProgressPage::OnSetActive() 
{
	if (!CWizardPage::OnSetActive())
		return FALSE;

	// Disable the buttons.
	SetWizardButtons(0);
	EnableCancelButton(FALSE);

	m_bCancel = FALSE;
	ASSERT(!m_taskThread);
	m_taskThread = NEW ImportSerialTaskThread(m_pSettings, this);
	m_taskThread->Start();

	return TRUE;
}

void CImportSerialProgressPage::OnTaskBegin()
{
	m_lastProgressTime = 0;	// Should actually be in OnTrackLogBegin, but that doesn't exist.
	PostMessage(WM_TASK_BEGIN);
}

LRESULT CImportSerialProgressPage::OnTaskBeginStub(WPARAM wParam, LPARAM lParam)
{
	CString s;
	s.Format(IDS_LOOKING_FOR_GARMIN, m_pSettings->GetUserSerialDeviceName().c_str());
	SetDlgItemText(IDC_TRACKLOG_PROGRESS, s);

	EnableCancelButton(TRUE);

	return 0;
}

void CImportSerialProgressPage::OnTrackLogProgress(int current, int maximum)
{
	if (GetTickCount() >= m_lastProgressTime + 500)
	{
		m_lastProgressTime = GetTickCount();
		PostMessage(WM_TRACKLOG_PROGRESS, current, maximum);
	}
}

LRESULT CImportSerialProgressPage::OnTrackLogProgressStub(WPARAM wParam, LPARAM lParam)
{
	int current = (int)wParam;
	int maximum = (int)lParam;

	m_progressCtrl.SetRange32(0, maximum);
	m_progressCtrl.SetPos(current);

	CString s;
	s.Format(IDS_DOWNLOADING_TRACKLOG, current, maximum);
	SetDlgItemText(IDC_TRACKLOG_PROGRESS, s);

	return 0;
}

void CImportSerialProgressPage::OnTrackLogComplete()
{
	m_lastProgressTime = 0;	// Should actually be in OnWaypointBegin, but that doesn't exist.
	PostMessage(WM_TRACKLOG_COMPLETE);
}

LRESULT CImportSerialProgressPage::OnTrackLogCompleteStub(WPARAM wParam, LPARAM lParam)
{
	return 0;
}

void CImportSerialProgressPage::OnWaypointProgress(int current, int maximum)
{
	if (GetTickCount() >= m_lastProgressTime + 500)
	{
		m_lastProgressTime = GetTickCount();
		PostMessage(WM_WAYPOINT_PROGRESS, current, maximum);
	}
}

LRESULT CImportSerialProgressPage::OnWaypointProgressStub(WPARAM wParam, LPARAM lParam)
{
	int current = (int)wParam;
	int maximum = (int)lParam;

	m_progressCtrl.SetRange32(0, maximum);
	m_progressCtrl.SetPos(current);

	CString s;
	s.Format(IDS_DOWNLOADING_WAYPOINTS, current, maximum);
	SetDlgItemText(IDC_TRACKLOG_PROGRESS, s);

	return 0;
}

void CImportSerialProgressPage::OnWaypointComplete()
{
	PostMessage(WM_WAYPOINT_COMPLETE);
}

LRESULT CImportSerialProgressPage::OnWaypointCompleteStub(WPARAM wParam, LPARAM lParam)
{
	return 0;
}

void CImportSerialProgressPage::OnTaskComplete(STATUS taskStatus, const Product_Data_Type *product, const PositionFixCollection *positions, const WaypointCollection *waypoints)
{	
	// We need to take a copy of the positions, because the threading makes life awkward.
	// We ought to use the Command pattern and stash the positions there, but
	// in the name of expedience...
	if (product)
		m_product = *product;

	m_positions.clear();
	if (positions)
	{
		m_positions.reserve(positions->size());
		std::copy(positions->begin(), positions->end(), std::back_inserter(m_positions));
	}
	m_waypoints.clear();
	if (waypoints)
	{
		m_waypoints.reserve(waypoints->size());
		std::copy(waypoints->begin(), waypoints->end(), std::back_inserter(m_waypoints));
	}

	PostMessage(WM_TASK_COMPLETE, 0, taskStatus);
}

LRESULT CImportSerialProgressPage::OnTaskCompleteStub(WPARAM wParam, LPARAM lParam)
{
	STATUS taskStatus = (STATUS)lParam;

	// Doing the Join() here requires that the background thread not call
	// SendMessage after it calls ReportComplete.  If it does, it'll deadlock.
	m_taskThread->Join();
	delete m_taskThread;
	m_taskThread = NULL;

	m_pResults->SetTaskStatus(taskStatus);
	m_pResults->SetProductData(m_product);
	m_pResults->ClearPositions();
	m_pResults->AddPositions(m_positions);
	m_pResults->ClearWaypoints();
	m_pResults->AddWaypoints(m_waypoints);

	static_cast<CPropertySheet *>(GetParent())->PressButton(PSBTN_NEXT);

	return 0;
}

bool CImportSerialProgressPage::QueryCancelTask()
{
	return m_bCancel ? true : false;
}

/** Called when the Cancel button is pressed.  Return FALSE to disallow cancellation.
 */
BOOL CImportSerialProgressPage::OnQueryCancel()
{
	m_bCancel = TRUE;
	return FALSE;
}
