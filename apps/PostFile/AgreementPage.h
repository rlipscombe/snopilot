#pragma once
#include "resource.h"
#include "win32_ui/WizardPage.h"

class PostFileSettings;

// CAgreementPage dialog
class CAgreementPage : public CWizardPage
{
	PostFileSettings *m_pSettings;
	CRichEditCtrl m_agreementCtrl;

	DECLARE_DYNAMIC(CAgreementPage)

public:
	CAgreementPage(PostFileSettings *pSettings);
	virtual ~CAgreementPage();

	virtual BOOL OnInitDialog();
	virtual BOOL OnSetActive();

// Dialog Data
	enum { IDD = IDD_AGREEMENT_PAGE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	afx_msg void OnBnClickedDoNotAgree();
	afx_msg void OnBnClickedAgree();

private:
	void EnableButtons();
};
