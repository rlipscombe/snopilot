// StartPageFrame.cpp : implementation file
//

#include "stdafx.h"
#include "StartPageFrame.h"

// CStartPageFrame

IMPLEMENT_DYNCREATE(CStartPageFrame, CMDIChildWnd)

CStartPageFrame::CStartPageFrame()
{
}

CStartPageFrame::~CStartPageFrame()
{
}


BEGIN_MESSAGE_MAP(CStartPageFrame, CMDIChildWnd)
END_MESSAGE_MAP()


// CStartPageFrame message handlers

void CStartPageFrame::ActivateFrame(int nCmdShow)
{
#if MAXIMIZE_MDI_CHILDREN
	if (GetMDIFrame()->MDIGetActive())
		CMDIChildWnd::ActivateFrame(nCmdShow);
	else
		CMDIChildWnd::ActivateFrame(SW_SHOWMAXIMIZED);
#else
	CMDIChildWnd::ActivateFrame(nCmdShow);
#endif
}
