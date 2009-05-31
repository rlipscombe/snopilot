#pragma once
#include "resource.h"
#include "win32_ui/WizardPage.h"

// CFilenamePage dialog
class PostFileSettings;
class CSnoTrackFileReaderCallback;

class CFilenamePage : public CWizardPage
{
	PostFileSettings *m_pSettings;
	
	DECLARE_DYNAMIC(CFilenamePage)

public:
	CFilenamePage(PostFileSettings *pSettings);
	virtual ~CFilenamePage();

	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardNext();

	// Dialog Data
	enum { IDD = IDD_FILENAME_PAGE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	afx_msg void OnBnClickedBrowse();

private:
	bool LoadStats(CSnoTrackFileReaderCallback *pCallback);
};
