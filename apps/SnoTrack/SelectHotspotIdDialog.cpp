// SelectHotspotIdDialog.cpp : implementation file
//

#include "stdafx.h"
#include "SelectHotspotIdDialog.h"

// CSelectHotspotIdDialog dialog

IMPLEMENT_DYNAMIC(CSelectHotspotIdDialog, CDialog)

CSelectHotspotIdDialog::CSelectHotspotIdDialog(int currentHotspotID, int nextHotspotID, CWnd* pParent /*=NULL*/)
	: CDialog(CSelectHotspotIdDialog::IDD, pParent), m_currentHotspotID(currentHotspotID), m_nextHotspotID(nextHotspotID)
{
}

CSelectHotspotIdDialog::~CSelectHotspotIdDialog()
{
}

void CSelectHotspotIdDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CSelectHotspotIdDialog, CDialog)
END_MESSAGE_MAP()

// CSelectHotspotIdDialog message handlers

BOOL CSelectHotspotIdDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	static_cast<CSpinButtonCtrl *>(GetDlgItem(IDC_SPIN_ID))->SetRange32(0, INT_MAX);

	SetDlgItemInt(IDC_CURRENT_ID, m_currentHotspotID);
	SetDlgItemInt(IDC_NEW_ID, m_nextHotspotID);

	GetDlgItem(IDC_NEW_ID)->SetFocus();
	return FALSE;
}

void CSelectHotspotIdDialog::OnOK()
{
	m_nextHotspotID = GetDlgItemInt(IDC_NEW_ID, NULL, TRUE);

	CDialog::OnOK();
}
