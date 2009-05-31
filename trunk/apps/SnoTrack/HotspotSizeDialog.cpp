// HotspotSizeDialog.cpp : implementation file
//

#include "stdafx.h"
#include "HotspotSizeDialog.h"
#include ".\hotspotsizedialog.h"

#if defined(ENABLE_DESIGN_MODE)

// CHotspotSizeDialog dialog

IMPLEMENT_DYNAMIC(CHotspotSizeDialog, CDialog)

CHotspotSizeDialog::CHotspotSizeDialog(int hotspotSize, CWnd* pParent /*=NULL*/)
	: CDialog(CHotspotSizeDialog::IDD, pParent)
	, m_hotspotSize(hotspotSize)
{
}

CHotspotSizeDialog::~CHotspotSizeDialog()
{
}

void CHotspotSizeDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_HOTSPOT_SIZE, m_hotspotSize);
	DDV_MinMaxInt(pDX, m_hotspotSize, 1, 10);
}


BEGIN_MESSAGE_MAP(CHotspotSizeDialog, CDialog)
END_MESSAGE_MAP()

// CHotspotSizeDialog message handlers

BOOL CHotspotSizeDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	static_cast<CSpinButtonCtrl *>(GetDlgItem(IDC_HOTSPOT_SIZE_SPIN))->SetRange32(1, 10);

	return TRUE;
}

#endif
