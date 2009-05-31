#pragma once
#include "resource.h"
#include "win32_ui/WizardPage.h"

class NewFileSettings;

// CNewFileCompletePage dialog

class CNewFileCompletePage : public CWizardPage
{
	const NewFileSettings *m_pSettings;

	DECLARE_DYNAMIC(CNewFileCompletePage)

public:
	CNewFileCompletePage(const NewFileSettings *pSettings);
	virtual ~CNewFileCompletePage();

	virtual BOOL OnInitDialog();
	virtual BOOL OnSetActive();

// Dialog Data
	enum { IDD = IDD_NEW_FILE_COMPLETE_PAGE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	afx_msg LRESULT OnDeferredInit(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
};
