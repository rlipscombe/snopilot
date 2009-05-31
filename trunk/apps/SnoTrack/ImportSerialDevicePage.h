/* ImportSerialDevicePage.h
 *
 * Copyright (C) 2003 Roger Lipscombe
 *     http://www.differentpla.net/~roger/
 */

#pragma once
#include "resource.h"
#include "win32_ui/WizardPage.h"

/////////////////////////////////////////////////////////////////////////////
// CImportSerialDevicePage dialog

class ImportSerialSettings;

class CImportSerialDevicePage : public CWizardPage
{
	ImportSerialSettings *m_pSettings;

    DECLARE_DYNAMIC(CImportSerialDevicePage)

public:
    CImportSerialDevicePage(ImportSerialSettings *pSettings);
    ~CImportSerialDevicePage();

    //{{AFX_DATA(CImportSerialDevicePage)
    enum { IDD = IDD_IMPORT_SERIAL_DEVICE_PAGE };
	CComboBox m_serialDevices;
    //}}AFX_DATA

    //{{AFX_VIRTUAL(CImportSerialDevicePage)
    public:
    virtual BOOL OnSetActive();
    virtual LRESULT OnWizardNext();
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

protected:
    //{{AFX_MSG(CImportSerialDevicePage)
    //}}AFX_MSG

    DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
};
