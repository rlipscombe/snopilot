// ViewOptionsDialog.cpp : implementation file
//

#include "stdafx.h"
#include "ViewOptionsDialog.h"

// CViewOptionsDialog dialog

IMPLEMENT_DYNAMIC(CViewOptionsDialog, CDialog)

CViewOptionsDialog::CViewOptionsDialog(Layers *layers, CWnd* pParent /*=NULL*/)
	: CDialog(CViewOptionsDialog::IDD, pParent), m_layers(layers)
{
}

CViewOptionsDialog::~CViewOptionsDialog()
{
}

void CViewOptionsDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_LISTCTRL, m_listCtrl);
}

BEGIN_MESSAGE_MAP(CViewOptionsDialog, CDialog)
END_MESSAGE_MAP()

// CViewOptionsDialog message handlers

BOOL CViewOptionsDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_listCtrl.InsertColumn(0, _T("Name"));
	CRect listRect;
	m_listCtrl.GetClientRect(&listRect);
	m_listCtrl.SetColumnWidth(0, listRect.Width() - 10);
	m_listCtrl.SetExtendedStyle(LVS_EX_CHECKBOXES | m_listCtrl.GetExtendedStyle());
	m_listCtrl.SetFocus();

	for (Layers::const_iterator i = m_layers->begin(); i != m_layers->end(); ++i)
	{
		Layer *layer = *i;
		
		if (!layer->IsSpecial())
		{
			int nItem = m_listCtrl.InsertItem(m_listCtrl.GetItemCount(), layer->GetName());
			m_listCtrl.SetCheck(nItem, layer->IsEnabled());
		}
	}

	return FALSE;
}

void CViewOptionsDialog::OnOK()
{
	// Verify that the user really wants to hide everything.
	bool anyEnabled = false;
	for (int nItem = 0; nItem < m_listCtrl.GetItemCount(); ++nItem)
	{
		BOOL bCheck = m_listCtrl.GetCheck(nItem);
		if (bCheck)
		{
			anyEnabled = true;
			break;
		}
	}

	if (!anyEnabled)
	{
		UINT nResult = AfxMessageBox(IDS_CONFIRM_HIDE_ALL_LAYERS, MB_YESNO);
		if (nResult == IDNO)
			return;
	}

	// Save the settings from the list control.
	int n = 0;
	for (Layers::const_iterator i = m_layers->begin(); i != m_layers->end(); ++i)
	{
		Layer *layer = *i;

		if (!layer->IsSpecial())
		{
			bool enable = m_listCtrl.GetCheck(n) ? true : false;
			layer->Enable(enable);
			++n;
		}
	}

	CDialog::OnOK();
}
