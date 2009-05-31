#pragma once
#include "resource.h"
#include "win32_ui/WizardPage.h"

class NewFileSettings;

class CNewFileResortPage : public CWizardPage
{
	NewFileSettings *m_pSettings;

	CEdit m_commentsEdit;

	DECLARE_DYNAMIC(CNewFileResortPage)

public:
	CNewFileResortPage(NewFileSettings *pSettings);
	virtual ~CNewFileResortPage();

	virtual BOOL OnInitDialog();
	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardNext();

// Dialog Data
	enum { IDD = IDD_NEW_FILE_RESORT_PAGE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	afx_msg void OnBnClickedCustomResort();
	afx_msg void OnBnClickedPredefinedResort();
	afx_msg void OnBnClickedBrowseResortFile();

	DECLARE_MESSAGE_MAP()

private:
	void EnableControls();
public:
	afx_msg void OnEnChangeResortFile();
	afx_msg void OnEnChangeResortName();
};
