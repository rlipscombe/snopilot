#pragma once
#include "resource.h"
#include "win32_ui/TextHyperlink.h"

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();
	virtual BOOL OnInitDialog();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };
	CTextHyperlink m_websiteUrl;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

