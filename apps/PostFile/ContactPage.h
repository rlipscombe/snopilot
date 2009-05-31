#pragma once
#include "resource.h"
#include "win32_ui/WizardPage.h"
#include "win32_ui/TextHyperlink.h"

class PostFileSettings;

// CContactPage dialog
class CContactPage : public CWizardPage
{
	PostFileSettings *m_pSettings;

	CTextHyperlink m_forumLink;

	DECLARE_DYNAMIC(CContactPage)

public:
	CContactPage(PostFileSettings *pSettings);
	virtual ~CContactPage();

	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardNext();

// Dialog Data
	enum { IDD = IDD_CONTACT_PAGE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	afx_msg void OnBnClickedAnonymous();
	
private:
	void EnableControls();
};
