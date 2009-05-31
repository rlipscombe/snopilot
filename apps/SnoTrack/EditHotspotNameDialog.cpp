// EditHotspotNameDialog.cpp : implementation file
//

#include "stdafx.h"
#include "EditHotspotNameDialog.h"
#include ".\edithotspotnamedialog.h"

// CEditHotspotNameDialog dialog

IMPLEMENT_DYNAMIC(CEditHotspotNameDialog, CDialog)

CEditHotspotNameDialog::CEditHotspotNameDialog(int hotspotID, CString hotspotName, CWnd* pParent /*=NULL*/)
	: CDialog(CEditHotspotNameDialog::IDD, pParent), m_hotspotID(hotspotID), m_hotspotName(hotspotName)
{
}

CEditHotspotNameDialog::~CEditHotspotNameDialog()
{
}

void CEditHotspotNameDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CEditHotspotNameDialog, CDialog)
END_MESSAGE_MAP()

// CEditHotspotNameDialog message handlers
BOOL CEditHotspotNameDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetDlgItemInt(IDC_HOTSPOT_ID, m_hotspotID);
	SetDlgItemText(IDC_HOTSPOT_NAME, m_hotspotName);

	GetDlgItem(IDC_HOTSPOT_NAME)->SetFocus();

	return FALSE;
}

void CEditHotspotNameDialog::OnOK()
{
	GetDlgItemText(IDC_HOTSPOT_NAME, m_hotspotName);

	CDialog::OnOK();
}
