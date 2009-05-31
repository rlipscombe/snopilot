// EditPathsDialog.cpp : implementation file
//

#include "stdafx.h"
#include "EditPathsDialog.h"
#include "EditPathDialog.h"

// CEditPathsDialog dialog

IMPLEMENT_DYNAMIC(CEditPathsDialog, CDialog)

CEditPathsDialog::CEditPathsDialog(const HotspotPathCollection &paths, CWnd* pParent /*=NULL*/)
	: CDialog(CEditPathsDialog::IDD, pParent), m_paths(paths)
{
}

CEditPathsDialog::~CEditPathsDialog()
{
}

void CEditPathsDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LISTCTRL, m_listCtrl);
}


BEGIN_MESSAGE_MAP(CEditPathsDialog, CDialog)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LISTCTRL, OnLvnItemchangedListctrl)
	ON_BN_CLICKED(IDC_EDIT_PATH, OnBnClickedEditPath)
	ON_BN_CLICKED(IDC_DELETE_PATH, OnBnClickedDeletePath)
	ON_BN_CLICKED(IDC_NEW_PATH, OnBnClickedNewPath)
	ON_NOTIFY(NM_DBLCLK, IDC_LISTCTRL, OnNMDblclkListctrl)
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CEditPathsDialog message handlers

BOOL CEditPathsDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_listCtrl.InsertColumn(0, _T("Path Name"), LVCFMT_LEFT, 120, 0);
	m_listCtrl.InsertColumn(1, _T("Hotspots"), LVCFMT_LEFT, 160, 1);

	CImageList *iml = new CImageList;
	VERIFY(iml->Create(IDB_EVENT_ICONS, 16, 1, RGB(255,0,255)));
	m_listCtrl.SetImageList(iml, LVSIL_SMALL);

	PopulateListCtrl();

	EnableControls();

	m_listCtrl.SetFocus();
	return FALSE;
}

void CEditPathsDialog::PopulateListCtrl()
{
	m_listCtrl.DeleteAllItems();

	for (HotspotPathCollection::const_iterator i = m_paths.begin(); i != m_paths.end(); ++i)
	{
		InsertListItem(*i);
	}
}

int CEditPathsDialog::InsertListItem(const HotspotPath &path)
{
	CString strPathName = ConvertUTF8ToSystem(path.GetName()).c_str();

	CString strPathHotspots;
	for (HotspotPath::const_iterator j = path.begin(); j != path.end(); ++j)
	{
		CString str;
		str.Format("%d", *j);

		if (!strPathHotspots.IsEmpty())
			strPathHotspots += ", ";
		strPathHotspots += str;
	}

	LVITEM item;
	memset(&item, 0, sizeof(LVITEM));
	item.mask = LVIF_TEXT | LVIF_IMAGE;
	item.iItem = m_listCtrl.GetItemCount();
	item.pszText = const_cast<TCHAR *>(LPCTSTR(strPathName));
	item.iImage = path.GetImageIndex();

	int nItem = m_listCtrl.InsertItem(&item);
	ASSERT(nItem != -1);

	item.iSubItem = 1;
	item.pszText = const_cast<TCHAR *>(LPCTSTR(strPathHotspots));

	VERIFY(m_listCtrl.SetItem(&item));

	return nItem;
}

void CEditPathsDialog::EnableControls()
{
	GetDlgItem(IDC_NEW_PATH)->EnableWindow(TRUE);
	GetDlgItem(IDC_EDIT_PATH)->EnableWindow(m_listCtrl.GetSelectedCount() == 1);
	GetDlgItem(IDC_DELETE_PATH)->EnableWindow(m_listCtrl.GetSelectedCount() >= 1);
}

void CEditPathsDialog::OnLvnItemchangedListctrl(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

	EnableControls();

	*pResult = 0;
}

void CEditPathsDialog::UpdateItem(int nItem)
{
	const HotspotPath &path = m_paths[nItem];
	CString strPathName = ConvertUTF8ToSystem(path.GetName()).c_str();

	CString strPathHotspots;
	for (HotspotPath::const_iterator j = path.begin(); j != path.end(); ++j)
	{
		CString str;
		str.Format("%d", *j);

		if (!strPathHotspots.IsEmpty())
			strPathHotspots += ", ";
		strPathHotspots += str;
	}

	LVITEM item;
	memset(&item, 0, sizeof(LVITEM));
	item.mask = LVIF_TEXT | LVIF_IMAGE;
	item.iItem = nItem;
	item.pszText = const_cast<TCHAR *>(LPCTSTR(strPathName));
	item.iImage = path.GetImageIndex();

	VERIFY(m_listCtrl.SetItem(&item));

	item.iSubItem = 1;
	item.pszText = const_cast<TCHAR *>(LPCTSTR(strPathHotspots));

	VERIFY(m_listCtrl.SetItem(&item));
}

void CEditPathsDialog::OnBnClickedEditPath()
{
	ASSERT(m_listCtrl.GetSelectedCount() == 1);

	int selectedItem = m_listCtrl.GetNextItem(-1, LVNI_SELECTED);
	ASSERT(selectedItem >= 0 && selectedItem < (int)m_paths.size());

	HotspotPath existingPath = m_paths[selectedItem];
	CEditPathDialog dlg(existingPath);
	if (dlg.DoModal() == IDOK)
	{
		m_paths[selectedItem] = dlg.GetHotspotPath();
		UpdateItem(selectedItem);
	}

	EnableControls();
}

void CEditPathsDialog::OnBnClickedDeletePath()
{
	ASSERT(m_listCtrl.GetSelectedCount() >= 1);

	int selectedItem = m_listCtrl.GetNextItem(-1, LVNI_SELECTED);
	ASSERT(selectedItem >= 0 && selectedItem < (int)m_paths.size());

	m_paths.erase(m_paths.begin() + selectedItem);
	m_listCtrl.DeleteItem(selectedItem);

	EnableControls();
}

void CEditPathsDialog::OnBnClickedNewPath()
{
	HotspotPath newPath("New Path", false, 0);
	CEditPathDialog dlg(newPath);
	if (dlg.DoModal() == IDOK)
	{
		HotspotPath newPath = dlg.GetHotspotPath();
		m_paths.push_back(newPath);
		int nItem = InsertListItem(newPath);
		
		// Select the new item, but not the others.
		for (int i = 0; i < m_listCtrl.GetItemCount(); ++i)
		{
			m_listCtrl.SetItemState(i, (nItem == i) ? LVIS_SELECTED : 0, LVIS_SELECTED);
		}
	}
	
	EnableControls();
}

void CEditPathsDialog::OnNMDblclkListctrl(NMHDR *pNMHDR, LRESULT *pResult)
{
	if (m_listCtrl.GetSelectedCount() == 1)
	{
		OnBnClickedEditPath();
	}

	EnableControls();
	*pResult = 0;
}

void CEditPathsDialog::OnDestroy()
{
	delete m_listCtrl.GetImageList(LVSIL_SMALL);

	CDialog::OnDestroy();
}
