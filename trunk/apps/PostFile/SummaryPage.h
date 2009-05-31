#pragma once
#include "resource.h"
#include "win32_ui/WizardPage.h"

class PostFileSettings;
class CSnoTrackFileReaderCallback;

// CSummaryPage dialog
class CSummaryPage : public CWizardPage
{
	CRichEditCtrl m_summaryCtrl;
	PostFileSettings *m_pSettings;

	DECLARE_DYNAMIC(CSummaryPage)

public:
	CSummaryPage(PostFileSettings *pSettings);
	virtual ~CSummaryPage();

	virtual BOOL OnSetActive();

// Dialog Data
	enum { IDD = IDD_SUMMARY_PAGE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	LRESULT OnDeferredInit(WPARAM wParam, LPARAM lParam);

private:
	void InitialiseSummaryText();
};
