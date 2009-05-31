// SnoTrackDLL.h : main header file for the SnoTrackDLL DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols


// CSnoTrackDLLApp
// See SnoTrackDLL.cpp for the implementation of this class
//

class CSnoTrackDLLApp : public CWinApp
{
public:
	CSnoTrackDLLApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
