/* WelcomePage.h
 *
 * Copyright (C) 2003 Roger Lipscombe
 *     http://www.differentpla.net/~roger/
 */

#pragma once
#include "resource.h"
#include "win32_ui/WizardPage.h"

/////////////////////////////////////////////////////////////////////////////
// CImportSerialWelcomePage dialog

class CImportSerialWelcomePage : public CWizardPage
{
    DECLARE_DYNCREATE(CImportSerialWelcomePage)

public:
    CImportSerialWelcomePage();
    ~CImportSerialWelcomePage();

    //{{AFX_DATA(CImportSerialWelcomePage)
    enum { IDD = IDD_IMPORT_SERIAL_WELCOME_PAGE };
    //}}AFX_DATA

    //{{AFX_VIRTUAL(CImportSerialWelcomePage)
    public:
    virtual BOOL OnSetActive();
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

protected:
    //{{AFX_MSG(CImportSerialWelcomePage)
    virtual BOOL OnInitDialog();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};
