/* CompletePage.h
 *
 * Copyright (C) 2003 Roger Lipscombe
 *     http://www.differentpla.net/~roger/
 */

#ifndef COMPLETE_PAGE_H
#define COMPLETE_PAGE_H 1

#include "resource.h"
#include "win32_ui/WizardPage.h"

/////////////////////////////////////////////////////////////////////////////
// CImportSerialCompletePage dialog

class ImportSerialSettings;
class ImportSerialResults;

class CImportSerialCompletePage : public CWizardPage
{
	const ImportSerialResults *m_pResults;

    DECLARE_DYNAMIC(CImportSerialCompletePage)

public:
    CImportSerialCompletePage(const ImportSerialResults *pResults);
    ~CImportSerialCompletePage();

    //{{AFX_DATA(CImportSerialCompletePage)
    enum { IDD = IDD_IMPORT_SERIAL_COMPLETE_PAGE };
    //}}AFX_DATA

    //{{AFX_VIRTUAL(CImportSerialCompletePage)
	public:
	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardBack();
	protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

protected:
    //{{AFX_MSG(CImportSerialCompletePage)
    virtual BOOL OnInitDialog();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

#endif /* COMPLETE_PAGE_H */
