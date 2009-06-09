// SnoTrack.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "SnoTrack.h"
#include "unit_tests.h"
#include "unix_time.h"
#include "MainFrm.h"

#include "AboutDlg.h"

#include "ChildFrm.h"
#include "SnoTrackDoc.h"
#include "CartesianView.h"

#include "StartPageFrame.h"
#include "StartPageDoc.h"
#include "StartPageView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CSnoTrackApp

BEGIN_MESSAGE_MAP(CSnoTrackApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
END_MESSAGE_MAP()


// CSnoTrackApp construction
CSnoTrackApp::CSnoTrackApp()
{
}

CSnoTrackApp theApp;

BOOL CSnoTrackApp::InitInstance()
{
	InitCommonControls();

	CWinApp::InitInstance();

	SetRegistryKey(_T("snopilot"));

#if 0
	if (runUnitTests())
	{
		int yn = AfxMessageBox("Unit Test(s) failed.  Continue?", MB_YESNO);
		if (yn != IDYES)
			return FALSE;
	}

	// Midnight, the morning of 1st January 2006.
	time_t expiry = MakeUnixTime(2006, 1, 1, 0, 0, 0);
	time_t now = time(NULL);
	if (now >= expiry)
	{
		AfxMessageBox(IDS_EXPIRED);
		return FALSE;
	}
#endif

	LoadStdProfileSettings(10);

	CMultiDocTemplate *pDocTemplate = new CMultiDocTemplate(IDR_SNOTRACKTYPE,
		RUNTIME_CLASS(CSnoTrackDoc),
		RUNTIME_CLASS(CChildFrame), // custom MDI child frame
		RUNTIME_CLASS(CCartesianView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);

	CMultiDocTemplate *pStartPageDocTemplate = new CMultiDocTemplate(IDR_STARTPAGEVIEW,
		RUNTIME_CLASS(CStartPageDoc),
		RUNTIME_CLASS(CStartPageFrame),
		RUNTIME_CLASS(CStartPageView));
	if (!pStartPageDocTemplate)
		return FALSE;
	AddDocTemplate(pStartPageDocTemplate);

	CMainFrame* pMainFrame = new CMainFrame;
	if (!pMainFrame || !pMainFrame->LoadFrame(IDR_MAINFRAME))
		return FALSE;
	m_pMainWnd = pMainFrame;

	m_pMainWnd->DragAcceptFiles();

	EnableShellOpen();
	RegisterShellFileTypes(TRUE);

	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);
	
	// If we've not been given any command line arguments, then display the start page instead.
	BOOL bDisplayStartPage = FALSE;
	if (cmdInfo.m_nShellCommand == CCommandLineInfo::FileNew)
	{
		cmdInfo.m_nShellCommand = CCommandLineInfo::FileNothing;
		bDisplayStartPage = TRUE;
	}

	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	if (bDisplayStartPage)
		pStartPageDocTemplate->OpenDocumentFile(NULL);

	pMainFrame->ShowWindow(m_nCmdShow);
	pMainFrame->UpdateWindow();
	return TRUE;
}

// App command to run the dialog
void CSnoTrackApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

// CSnoTrackApp message handlers

