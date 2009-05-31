// EditLiftsDialog.cpp : implementation file
//

#include "stdafx.h"
#include "EditLiftsDialog.h"
#include "LatLon.h"
#include "EditLiftDialog.h"

// CEditLiftsDialog dialog

IMPLEMENT_DYNAMIC(CEditLiftsDialog, CDialog)

CEditLiftsDialog::CEditLiftsDialog(const LiftCollection &lifts, Projection *proj, CWnd* pParent /*=NULL*/)
	: CDialog(CEditLiftsDialog::IDD, pParent), m_lifts(lifts), m_proj(proj)
{
}

CEditLiftsDialog::~CEditLiftsDialog()
{
}

void CEditLiftsDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LISTCTRL, m_listCtrl);
}

void CEditLiftsDialog::GetLifts(LiftCollection *lifts)
{
	*lifts = m_lifts;
}

BEGIN_MESSAGE_MAP(CEditLiftsDialog, CDialog)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LISTCTRL, OnLvnItemchangedListctrl)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_DELETE, OnBnClickedDelete)
	ON_BN_CLICKED(IDC_EDIT, OnBnClickedEdit)
	ON_BN_CLICKED(IDC_NEW, OnBnClickedNew)
	ON_NOTIFY(NM_DBLCLK, IDC_LISTCTRL, OnNMDblclkListctrl)
END_MESSAGE_MAP()


// CEditLiftsDialog message handlers

BOOL CEditLiftsDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_listCtrl.InsertColumn(0, _T("Lift Name"), LVCFMT_LEFT, 120, 0);
	m_listCtrl.InsertColumn(1, _T("Bottom"), LVCFMT_LEFT, 160, 1);
	m_listCtrl.InsertColumn(2, _T("Top"), LVCFMT_LEFT, 160, 2);

	CImageList *iml = new CImageList;
	VERIFY(iml->Create(IDB_EVENT_ICONS, 16, 1, RGB(255,0,255)));
	m_listCtrl.SetImageList(iml, LVSIL_SMALL);

	PopulateListCtrl();

	EnableControls();

	m_listCtrl.SetFocus();
	return FALSE;
}

void CEditLiftsDialog::PopulateListCtrl()
{
	for (LiftCollection::const_iterator i = m_lifts.begin(); i != m_lifts.end(); ++i)
	{
		InsertListItem(*i);
	}
}

int CEditLiftsDialog::InsertListItem(const Lift &lift)
{
	CString strName = ConvertUTF8ToSystem(lift.GetLabel()).c_str();
	CString strBottom = FormatLatLonPosition(lift.GetBottomLatitude(), lift.GetBottomLongitude());
	CString strTop = FormatLatLonPosition(lift.GetTopLatitude(), lift.GetTopLongitude());

	LVITEM item;
	memset(&item, 0, sizeof(LVITEM));
	item.mask = LVIF_TEXT | LVIF_IMAGE;
	item.iItem = m_listCtrl.GetItemCount();
	item.pszText = const_cast<TCHAR *>(LPCTSTR(strName));
	item.iImage = 1;

	int nItem = m_listCtrl.InsertItem(&item);
	ASSERT(nItem != -1);

	item.iSubItem = 1;
	item.pszText = const_cast<TCHAR *>(LPCTSTR(strBottom));

	VERIFY(m_listCtrl.SetItem(&item));

	item.iSubItem = 2;
	item.pszText = const_cast<TCHAR *>(LPCTSTR(strTop));

	VERIFY(m_listCtrl.SetItem(&item));

	return nItem;
}

void CEditLiftsDialog::UpdateItem(int nItem)
{
	const Lift &lift = m_lifts[nItem];

	CString strName = ConvertUTF8ToSystem(lift.GetLabel()).c_str();
	CString strBottom = FormatLatLonPosition(lift.GetBottomLatitude(), lift.GetBottomLongitude());
	CString strTop = FormatLatLonPosition(lift.GetTopLatitude(), lift.GetTopLongitude());

	LVITEM item;
	memset(&item, 0, sizeof(LVITEM));
	item.mask = LVIF_TEXT | LVIF_IMAGE;
	item.iItem = nItem;
	item.pszText = const_cast<TCHAR *>(LPCTSTR(strName));
	item.iImage = 1;

	VERIFY(m_listCtrl.SetItem(&item));

	item.iSubItem = 1;
	item.pszText = const_cast<TCHAR *>(LPCTSTR(strBottom));

	VERIFY(m_listCtrl.SetItem(&item));

	item.iSubItem = 2;
	item.pszText = const_cast<TCHAR *>(LPCTSTR(strTop));

	VERIFY(m_listCtrl.SetItem(&item));
}

void CEditLiftsDialog::EnableControls()
{
	GetDlgItem(IDC_NEW)->EnableWindow(TRUE);
	GetDlgItem(IDC_EDIT)->EnableWindow(m_listCtrl.GetSelectedCount() == 1);
	GetDlgItem(IDC_DELETE)->EnableWindow(m_listCtrl.GetSelectedCount() >= 1);
}

void CEditLiftsDialog::OnLvnItemchangedListctrl(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

	EnableControls();

	*pResult = 0;
}

void CEditLiftsDialog::OnDestroy()
{
	delete m_listCtrl.GetImageList(LVSIL_SMALL);

	CDialog::OnDestroy();
}

void CEditLiftsDialog::OnBnClickedDelete()
{
	ASSERT(m_listCtrl.GetSelectedCount() >= 1);

	int selectedItem = m_listCtrl.GetNextItem(-1, LVNI_SELECTED);
	ASSERT(selectedItem >= 0 && selectedItem < (int)m_lifts.size());

	m_lifts.erase(m_lifts.begin() + selectedItem);
	m_listCtrl.DeleteItem(selectedItem);

	EnableControls();
}

void CEditLiftsDialog::OnBnClickedEdit()
{
	ASSERT(m_listCtrl.GetSelectedCount() == 1);

	int selectedItem = m_listCtrl.GetNextItem(-1, LVNI_SELECTED);
	ASSERT(selectedItem >= 0 && selectedItem < (int)m_lifts.size());

	Lift existingLift = m_lifts[selectedItem];
	CEditLiftDialog dlg(existingLift, m_proj);
	if (dlg.DoModal() == IDOK)
	{
		m_lifts[selectedItem] = dlg.GetLift();
		UpdateItem(selectedItem);
	}

	EnableControls();
}

void CEditLiftsDialog::OnBnClickedNew()
{
	Lift newLift("New Lift", 0, 0, 0, 0, 0, 0, 0, 0);
	CEditLiftDialog dlg(newLift, m_proj);
	if (dlg.DoModal() == IDOK)
	{
		Lift newLift = dlg.GetLift();
		m_lifts.push_back(newLift);
		int nItem = InsertListItem(newLift);
		
		// Select the new item, but not the others.
		for (int i = 0; i < m_listCtrl.GetItemCount(); ++i)
		{
			m_listCtrl.SetItemState(i, (nItem == i) ? LVIS_SELECTED : 0, LVIS_SELECTED);
		}
	}
	
	EnableControls();
}

void CEditLiftsDialog::OnNMDblclkListctrl(NMHDR *pNMHDR, LRESULT *pResult)
{
	if (m_listCtrl.GetSelectedCount() == 1)
	{
		OnBnClickedEdit();
	}

	EnableControls();
	*pResult = 0;
}
