/* WelcomePage.h
 *
 * Copyright (C) 2003-2004 Roger Lipscombe
 *     http://www.differentpla.net/
 */

#ifndef __WELCOMEPAGE_H__
#define __WELCOMEPAGE_H__

#include "resource.h"
#include "win32_ui/WizardPage.h"

/////////////////////////////////////////////////////////////////////////////
// CWelcomePage dialog
class PostFileSettings;

class CWelcomePage : public CWizardPage
{
    DECLARE_DYNAMIC(CWelcomePage)

public:
    CWelcomePage();
    ~CWelcomePage();

    //{{AFX_DATA(CWelcomePage)
    enum { IDD = IDD_WELCOME_PAGE };
    //}}AFX_DATA

    //{{AFX_VIRTUAL(CWelcomePage)
    public:
    virtual BOOL OnSetActive();
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

protected:
    //{{AFX_MSG(CWelcomePage)
    virtual BOOL OnInitDialog();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

#endif // __WELCOMEPAGE_H__
