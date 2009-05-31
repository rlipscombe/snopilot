#pragma once

#include "progress.h"
#include "resource.h"

// CImportFileProgressDialog dialog
class CImportFileProgressDialog : public CDialog, public Progress
{
	BOOL m_bParentDisabled;	///< Did we disable our parent window?
	BOOL m_bCancel;			///< Has the user requested that we cancel whatever we're doing?

	DECLARE_DYNAMIC(CImportFileProgressDialog)

public:
	CImportFileProgressDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CImportFileProgressDialog();

	BOOL Create(CWnd *pParent = NULL);
	BOOL DestroyWindow();

// Dialog Data
	enum { IDD = IDD_IMPORT_FILE_PROGRESS };

// Progress
public:
	virtual STATUS OnProgress(int num, int denom);
	virtual STATUS OnComplete();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	void ReEnableParent();
	void PumpMessages();

	DECLARE_MESSAGE_MAP()
private:
	CProgressCtrl m_progressCtrl;
};
