#pragma once


// CStartPageFrame frame

class CStartPageFrame : public CMDIChildWnd
{
	DECLARE_DYNCREATE(CStartPageFrame)
protected:
	CStartPageFrame();           // protected constructor used by dynamic creation
	virtual ~CStartPageFrame();

protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual void ActivateFrame(int nCmdShow = -1);
};


