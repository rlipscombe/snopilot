#pragma once
#include "resource.h"
#include "win32_ui/WizardPage.h"

// CCompletePage dialog
class PostFileSettings;
class PostFileResults;

class CCompletePage : public CWizardPage
{
	PostFileSettings *m_pSettings;
	PostFileResults *m_pResults;

	DECLARE_DYNAMIC(CCompletePage)

public:
	CCompletePage(PostFileSettings *pSettings, PostFileResults *pResults);
	virtual ~CCompletePage();

	virtual BOOL OnInitDialog();
	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardBack();

// Dialog Data
	enum { IDD = IDD_COMPLETE_PAGE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	afx_msg void OnBnClickedDetails();

	DECLARE_MESSAGE_MAP()
};
