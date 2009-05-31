/* ImportSerialDevicePage.cpp
 *
 * Copyright (C) 2003 Roger Lipscombe
 *     http://www.differentpla.net/~roger/
 * Copyright (C) 2004 Snopilot
 *     http://www.snopilot.com/
 */

#include "StdAfx.h"
#include "ImportSerialDevicePage.h"
#include "ImportSerialSettings.h"

IMPLEMENT_DYNAMIC(CImportSerialDevicePage, CWizardPage)

/////////////////////////////////////////////////////////////////////////////
// CImportSerialDevicePage property page

CImportSerialDevicePage::CImportSerialDevicePage(ImportSerialSettings *pSettings)
    : CWizardPage(CImportSerialDevicePage::IDD, IDS_IMPORT_SERIAL_WIZARD_CAPTION, IDS_IMPORT_SERIAL_DEVICE_PAGE_TITLE, (UINT)0),
		m_pSettings(pSettings)
{
    //{{AFX_DATA_INIT(CImportSerialDevicePage)
    //}}AFX_DATA_INIT
}

CImportSerialDevicePage::~CImportSerialDevicePage()
{
}

void CImportSerialDevicePage::DoDataExchange(CDataExchange* pDX)
{
	CWizardPage::DoDataExchange(pDX);

	//{{AFX_DATA_MAP(CImportSerialDevicePage)
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_SERIAL_DEVICES, m_serialDevices);

	if (pDX->m_bSaveAndValidate)
	{
		int n = m_serialDevices.GetCurSel();
		CString s;
		s.Format(_T("COM%d"), n+1);
		m_pSettings->SetSerialDevice(tstring(s));
	}
}

BEGIN_MESSAGE_MAP(CImportSerialDevicePage, CWizardPage)
    //{{AFX_MSG_MAP(CImportSerialDevicePage)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CImportSerialDevicePage::OnSetActive() 
{
	if (!CWizardPage::OnSetActive())
		return FALSE;

	SetWizardButtons(PSWIZB_BACK|PSWIZB_NEXT);

	return TRUE;
}

LRESULT CImportSerialDevicePage::OnWizardNext() 
{
	if (!UpdateData(TRUE))
		return -1;

	return CWizardPage::OnWizardNext();
}

BOOL CImportSerialDevicePage::OnInitDialog()
{
	CWizardPage::OnInitDialog();

	/// @todo Use the SetupDi stuff to enumerate available COM ports?
	for (int i = 1; i <= 6; ++i)
	{
		/// @todo The combo should use the full names if possible (e.g. "Communications Port (COM1)" or "Port 1 on USB Adapter (COM3)").
		CString s;
		s.Format(_T("COM%d"), i);
		m_serialDevices.AddString(s);
	}

	/// @todo The combo should select the serial device that was last used.
	int n = m_serialDevices.FindString(-1, m_pSettings->GetUserSerialDeviceName().c_str());
	m_serialDevices.SetCurSel(n);
	m_serialDevices.SetFocus();

	return FALSE;
}
