// PostFile.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "PostFile.h"
#include "PostFileWizard.h"
#include "unix_time.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CPostFileApp

BEGIN_MESSAGE_MAP(CPostFileApp, CWinApp)
//	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()


// CPostFileApp construction

CPostFileApp::CPostFileApp()
{
}


// The one and only CPostFileApp object

CPostFileApp theApp;


// CPostFileApp initialization

BOOL CPostFileApp::InitInstance()
{
	AfxInitRichEdit2();

	InitCommonControls();

	CWinApp::InitInstance();

	AfxEnableControlContainer();

	SetRegistryKey(_T("snopilot"));

	// Midnight, the morning of 14th July, 2004.
	time_t expiry = MakeUnixTime(2004, 7, 14, 0, 0, 0);
	time_t now = time(NULL);
	if (now >= expiry)
	{
		AfxMessageBox(IDS_EXPIRED);
		return FALSE;
	}

	LPCTSTR lpszLocalFile = NULL;

	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);
	if (cmdInfo.m_nShellCommand == CCommandLineInfo::FileOpen)
		lpszLocalFile = cmdInfo.m_strFileName;

	INT_PTR nResponse = CPostFileWizard::DoWizard(lpszLocalFile, &m_pMainWnd);
	if (nResponse == IDOK)
	{
	}
	else if (nResponse == IDCANCEL)
	{
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}
