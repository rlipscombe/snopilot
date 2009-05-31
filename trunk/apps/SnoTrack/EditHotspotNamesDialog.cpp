// EditHotspotNamesDialog.cpp : implementation file
//

#include "stdafx.h"
#include "EditHotspotNamesDialog.h"
#include "EditHotspotNameDialog.h"
#include <algorithm>

// CEditHotspotNamesDialog dialog
IMPLEMENT_DYNAMIC(CEditHotspotNamesDialog, CDialog)

class DetailsCollector : public HotspotGridCallback
{
	HotspotDetailsVec *m_vec;
	const HotspotNames &m_hotspotNames;

public:
	DetailsCollector(HotspotDetailsVec *vec, const HotspotNames &hotspotNames)
		: m_vec(vec), m_hotspotNames(hotspotNames)
	{
	}

public:
	struct FindByID
	{
		int m_hotspotID;

		FindByID(int hotspotID)
			: m_hotspotID(hotspotID)
		{
		}

		bool operator() (const HotspotDetails &d) const
		{
			return d.hotspotID == m_hotspotID;
		}
	};

	virtual void OnHotspot(int xCell, int yCell, int hotspotId, CRect logicalRect)
	{
		// If the item's not already in our vector.
		HotspotDetailsVec::const_iterator i = std::find_if(m_vec->begin(), m_vec->end(), FindByID(hotspotId));
		if (i != m_vec->end())
			return;		// Already in there.

		HotspotDetails d;
		d.hotspotID = hotspotId;

		HotspotNames::const_iterator j = m_hotspotNames.find(hotspotId);
		if (j != m_hotspotNames.end())
			d.hotspotName = ConvertUTF8ToSystem(j->second).c_str();
		else
			d.hotspotName = _T("");

		m_vec->push_back(d);
	}
};

struct HotspotDetailsLtByID
{
	bool operator() (const HotspotDetails &lhs, const HotspotDetails &rhs) const
	{
		return lhs.hotspotID < rhs.hotspotID;
	}
};

CEditHotspotNamesDialog::CEditHotspotNamesDialog(const HotspotGrid *grid, const HotspotNames &hotspotNames, CWnd* pParent /*=NULL*/)
: CDialog(CEditHotspotNamesDialog::IDD, pParent)
{
	DetailsCollector collectDetails(&m_hotspotDetailsVec, hotspotNames);
	grid->EnumerateHotspots(&collectDetails);
	std::sort(m_hotspotDetailsVec.begin(), m_hotspotDetailsVec.end(), HotspotDetailsLtByID());
}

CEditHotspotNamesDialog::~CEditHotspotNamesDialog()
{
}

void CEditHotspotNamesDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LISTCTRL, m_listCtrl);
}


BEGIN_MESSAGE_MAP(CEditHotspotNamesDialog, CDialog)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LISTCTRL, OnLvnItemchangedListctrl)
	ON_NOTIFY(NM_DBLCLK, IDC_LISTCTRL, OnNMDblclkListctrl)
	ON_WM_DESTROY()
END_MESSAGE_MAP()

// CEditHotspotNamesDialog message handlers
BOOL CEditHotspotNamesDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_listCtrl.InsertColumn(0, _T("ID"), LVCFMT_LEFT, 120, 0);
	m_listCtrl.InsertColumn(1, _T("Name"), LVCFMT_LEFT, 160, 1);

	CImageList *iml = new CImageList;
	VERIFY(iml->Create(IDB_EVENT_ICONS, 16, 1, RGB(255,0,255)));
	m_listCtrl.SetImageList(iml, LVSIL_SMALL);

	PopulateListCtrl();

	EnableControls();

	m_listCtrl.SetFocus();
	return FALSE;
}

void CEditHotspotNamesDialog::PopulateListCtrl()
{
	m_listCtrl.DeleteAllItems();

	for (HotspotDetailsVec::const_iterator i = m_hotspotDetailsVec.begin(); i != m_hotspotDetailsVec.end(); ++i)
	{
		InsertListItem(*i);
	}
}

int CEditHotspotNamesDialog::InsertListItem(const HotspotDetails &d)
{
	CString strID;
	strID.Format("%d", d.hotspotID);

	CString strName = d.hotspotName;

	LVITEM item;
	memset(&item, 0, sizeof(LVITEM));
	item.mask = LVIF_TEXT | LVIF_IMAGE;
	item.iItem = m_listCtrl.GetItemCount();
	item.pszText = const_cast<TCHAR *>(LPCTSTR(strID));
	item.iImage = 0;

	int nItem = m_listCtrl.InsertItem(&item);
	ASSERT(nItem != -1);

	item.iSubItem = 1;
	item.pszText = const_cast<TCHAR *>(LPCTSTR(strName));

	VERIFY(m_listCtrl.SetItem(&item));

	return nItem;
}

void CEditHotspotNamesDialog::EnableControls()
{
	GetDlgItem(IDC_EDIT_NAME)->EnableWindow(m_listCtrl.GetSelectedCount() == 1);
}

void CEditHotspotNamesDialog::UpdateItem(int nItem)
{
	const HotspotDetails &d = m_hotspotDetailsVec[nItem];

	CString strID;
	strID.Format("%d", d.hotspotID);

	CString strName = d.hotspotName;

	LVITEM item;
	memset(&item, 0, sizeof(LVITEM));
	item.mask = LVIF_TEXT | LVIF_IMAGE;
	item.iItem = nItem;
	item.pszText = const_cast<TCHAR *>(LPCTSTR(strID));
	item.iImage = 0;

	VERIFY(m_listCtrl.SetItem(&item));

	item.iSubItem = 1;
	item.pszText = const_cast<TCHAR *>(LPCTSTR(strName));

	VERIFY(m_listCtrl.SetItem(&item));
}

void CEditHotspotNamesDialog::OnLvnItemchangedListctrl(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

	EnableControls();

	*pResult = 0;
}

void CEditHotspotNamesDialog::OnBnClickedEditName()
{
	ASSERT(m_listCtrl.GetSelectedCount() == 1);

	int selectedItem = m_listCtrl.GetNextItem(-1, LVNI_SELECTED);
	ASSERT(selectedItem >= 0 && selectedItem < (int)m_hotspotDetailsVec.size());

	HotspotDetails d = m_hotspotDetailsVec[selectedItem];
	CEditHotspotNameDialog dlg(d.hotspotID, d.hotspotName);
	if (dlg.DoModal() == IDOK)
	{
		m_hotspotDetailsVec[selectedItem].hotspotName = dlg.GetHotspotName();
		UpdateItem(selectedItem);
	}

	EnableControls();
}

void CEditHotspotNamesDialog::OnNMDblclkListctrl(NMHDR *pNMHDR, LRESULT *pResult)
{
	if (m_listCtrl.GetSelectedCount() == 1)
	{
		OnBnClickedEditName();
	}

	EnableControls();
	*pResult = 0;
}

void CEditHotspotNamesDialog::OnDestroy()
{
	delete m_listCtrl.GetImageList(LVSIL_SMALL);

	CDialog::OnDestroy();
}

HotspotNames CEditHotspotNamesDialog::GetHotspotNames() const
{
	HotspotNames names;

	for (HotspotDetailsVec::const_iterator i = m_hotspotDetailsVec.begin(); i != m_hotspotDetailsVec.end(); ++i)
	{
		names[i->hotspotID] = i->hotspotName;
	}

	return names;
}
