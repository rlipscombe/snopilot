// EditWaypointsDialog.cpp : implementation file
//

#include "stdafx.h"
#include "EditWaypointsDialog.h"
#include "LatLon.h"
#include "EditWaypointDialog.h"

// CEditWaypointsDialog dialog

IMPLEMENT_DYNAMIC(CEditWaypointsDialog, CDialog)

CEditWaypointsDialog::CEditWaypointsDialog(const WaypointCollection &waypoints, Projection *proj, CWnd* pParent /*=NULL*/)
	: CDialog(CEditWaypointsDialog::IDD, pParent), m_waypoints(waypoints), m_proj(proj)
{
}

CEditWaypointsDialog::~CEditWaypointsDialog()
{
}

void CEditWaypointsDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LISTCTRL, m_listCtrl);
}


BEGIN_MESSAGE_MAP(CEditWaypointsDialog, CDialog)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LISTCTRL, OnLvnItemchangedListctrl)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_DELETE, OnBnClickedDelete)
	ON_BN_CLICKED(IDC_EDIT, OnBnClickedEdit)
	ON_BN_CLICKED(IDC_NEW, OnBnClickedNew)
	ON_NOTIFY(NM_DBLCLK, IDC_LISTCTRL, OnNMDblclkListctrl)
END_MESSAGE_MAP()

// CEditWaypointsDialog message handlers
BOOL CEditWaypointsDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_listCtrl.InsertColumn(0, _T("Waypoint Name"), LVCFMT_LEFT, 120, 0);
	m_listCtrl.InsertColumn(1, _T("Longitude"), LVCFMT_LEFT, 160, 1);
	m_listCtrl.InsertColumn(2, _T("Latitude"), LVCFMT_LEFT, 160, 2);

//	CImageList *iml = new CImageList;
//	VERIFY(iml->Create(IDB_EVENT_ICONS, 16, 1, RGB(255,0,255)));
//	m_listCtrl.SetImageList(iml, LVSIL_SMALL);

	PopulateListCtrl();

	EnableControls();

	m_listCtrl.SetFocus();
	return FALSE;
}

void CEditWaypointsDialog::PopulateListCtrl()
{
	for (WaypointCollection::const_iterator i = m_waypoints.begin(); i != m_waypoints.end(); ++i)
	{
		InsertListItem(*i);
	}
}

int CEditWaypointsDialog::InsertListItem(const Waypoint &waypoint)
{
	CString strName = ConvertUTF8ToSystem(waypoint.GetLabel()).c_str();
	CString strLongitude = FormatLongitude(waypoint.GetLongitude(), FORMAT_LON_GARMIN);
	CString strLatitude = FormatLatitude(waypoint.GetLatitude(), FORMAT_LAT_GARMIN);

	LVITEM item;
	memset(&item, 0, sizeof(LVITEM));
	item.mask = LVIF_TEXT | LVIF_IMAGE;
	item.iItem = m_listCtrl.GetItemCount();
	item.pszText = const_cast<TCHAR *>(LPCTSTR(strName));
	item.iImage = 1;

	int nItem = m_listCtrl.InsertItem(&item);
	ASSERT(nItem != -1);

	item.iSubItem = 1;
	item.pszText = const_cast<TCHAR *>(LPCTSTR(strLongitude));

	VERIFY(m_listCtrl.SetItem(&item));

	item.iSubItem = 2;
	item.pszText = const_cast<TCHAR *>(LPCTSTR(strLatitude));

	VERIFY(m_listCtrl.SetItem(&item));

	return nItem;
}

void CEditWaypointsDialog::UpdateItem(int nItem)
{
	const Waypoint &waypoint = m_waypoints[nItem];

	CString strName = ConvertUTF8ToSystem(waypoint.GetLabel()).c_str();
	CString strLongitude = FormatLongitude(waypoint.GetLongitude(), FORMAT_LON_GARMIN);
	CString strLatitude = FormatLatitude(waypoint.GetLatitude(), FORMAT_LAT_GARMIN);

	LVITEM item;
	memset(&item, 0, sizeof(LVITEM));
	item.mask = LVIF_TEXT | LVIF_IMAGE;
	item.iItem = nItem;
	item.pszText = const_cast<TCHAR *>(LPCTSTR(strName));
	item.iImage = 1;

	VERIFY(m_listCtrl.SetItem(&item));

	item.iSubItem = 1;
	item.pszText = const_cast<TCHAR *>(LPCTSTR(strLongitude));

	VERIFY(m_listCtrl.SetItem(&item));

	item.iSubItem = 2;
	item.pszText = const_cast<TCHAR *>(LPCTSTR(strLatitude));

	VERIFY(m_listCtrl.SetItem(&item));
}

void CEditWaypointsDialog::EnableControls()
{
	GetDlgItem(IDC_NEW)->EnableWindow(TRUE);
	GetDlgItem(IDC_EDIT)->EnableWindow(m_listCtrl.GetSelectedCount() == 1);
	GetDlgItem(IDC_DELETE)->EnableWindow(m_listCtrl.GetSelectedCount() >= 1);
}

void CEditWaypointsDialog::OnLvnItemchangedListctrl(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

	EnableControls();

	*pResult = 0;
}

void CEditWaypointsDialog::OnDestroy()
{
	delete m_listCtrl.GetImageList(LVSIL_SMALL);

	CDialog::OnDestroy();
}

void CEditWaypointsDialog::OnBnClickedDelete()
{
	ASSERT(m_listCtrl.GetSelectedCount() >= 1);

	int selectedItem = m_listCtrl.GetNextItem(-1, LVNI_SELECTED);
	ASSERT(selectedItem >= 0 && selectedItem < (int)m_waypoints.size());

	m_waypoints.erase(m_waypoints.begin() + selectedItem);
	m_listCtrl.DeleteItem(selectedItem);

	EnableControls();
}

void CEditWaypointsDialog::OnBnClickedEdit()
{
	ASSERT(m_listCtrl.GetSelectedCount() == 1);

	int selectedItem = m_listCtrl.GetNextItem(-1, LVNI_SELECTED);
	ASSERT(selectedItem >= 0 && selectedItem < (int)m_waypoints.size());

	Waypoint existingWaypoint = m_waypoints[selectedItem];
	CEditWaypointDialog dlg(existingWaypoint, m_proj);
	if (dlg.DoModal() == IDOK)
	{
		m_waypoints[selectedItem] = dlg.GetWaypoint();
		UpdateItem(selectedItem);
	}

	EnableControls();
}

void CEditWaypointsDialog::OnBnClickedNew()
{
	Waypoint newWaypoint("New Waypoint", 0, 0, 0, 0);
	CEditWaypointDialog dlg(newWaypoint, m_proj);
	if (dlg.DoModal() == IDOK)
	{
		Waypoint newWaypoint = dlg.GetWaypoint();
		m_waypoints.push_back(newWaypoint);
		int nItem = InsertListItem(newWaypoint);
		
		// Select the new item, but not the others.
		for (int i = 0; i < m_listCtrl.GetItemCount(); ++i)
		{
			m_listCtrl.SetItemState(i, (nItem == i) ? LVIS_SELECTED : 0, LVIS_SELECTED);
		}
	}

	EnableControls();
}

void CEditWaypointsDialog::OnNMDblclkListctrl(NMHDR *pNMHDR, LRESULT *pResult)
{
	if (m_listCtrl.GetSelectedCount() == 1)
	{
		OnBnClickedEdit();
	}

	EnableControls();
	*pResult = 0;
}
