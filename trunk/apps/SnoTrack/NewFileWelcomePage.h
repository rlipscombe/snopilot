#pragma once
#include "resource.h"
#include "win32_ui/WizardPage.h"

// CNewFileWelcomePage dialog

class CNewFileWelcomePage : public CWizardPage
{
	DECLARE_DYNAMIC(CNewFileWelcomePage)

public:
	CNewFileWelcomePage();
	virtual ~CNewFileWelcomePage();

	virtual BOOL OnInitDialog();
	virtual BOOL OnSetActive();

// Dialog Data
	enum { IDD = IDD_NEW_FILE_WELCOME_PAGE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
