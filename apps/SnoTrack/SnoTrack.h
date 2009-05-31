// SnoTrack.h : main header file for the SnoTrack application
//
#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols


// CSnoTrackApp:
// See SnoTrack.cpp for the implementation of this class
//

class CSnoTrackApp : public CWinApp
{
public:
	CSnoTrackApp();

	virtual BOOL InitInstance();

	CRecentFileList *GetRecentFileList() { return m_pRecentFileList; }

	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CSnoTrackApp theApp;