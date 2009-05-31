// WalkHotspotsDialog.cpp : implementation file
//

#include "stdafx.h"
#include "WalkHotspotsDialog.h"
#include "win32/TimeZoneInformation.h"
#include "HotspotVisit.h"
#include ".\walkhotspotsdialog.h"

// CWalkHotspotsDialog dialog

IMPLEMENT_DYNAMIC(CWalkHotspotsDialog, CDialog)

CWalkHotspotsDialog::CWalkHotspotsDialog(const TIME_ZONE_INFORMATION *pTimeZoneInformation, const PositionFixCollection &positions, const HotspotGrid *pHotspotGrid, CWnd* pParent /*=NULL*/)
	: CDialog(CWalkHotspotsDialog::IDD, pParent), m_tzi(*pTimeZoneInformation), m_positions(positions), m_pHotspotGrid(pHotspotGrid)
{
}

CWalkHotspotsDialog::~CWalkHotspotsDialog()
{
}

void CWalkHotspotsDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LISTCTRL, m_listCtrl);
}


BEGIN_MESSAGE_MAP(CWalkHotspotsDialog, CDialog)
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CWalkHotspotsDialog message handlers

BOOL CWalkHotspotsDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	CRect rect;
	m_listCtrl.GetWindowRect(&rect);

	m_listCtrl.InsertColumn(0, _T("ID"), LVCFMT_LEFT, 40, 0);
	m_listCtrl.InsertColumn(1, _T("Entry"), LVCFMT_LEFT, (rect.Width() - 45) / 3, 1);
	m_listCtrl.InsertColumn(2, _T("Exit"), LVCFMT_LEFT, (rect.Width() - 45) / 3, 2);
	m_listCtrl.InsertColumn(3, _T("Elapsed"), LVCFMT_LEFT, (rect.Width() - 45) / 3, 3);

	CImageList *piml = new CImageList;
	VERIFY(piml->Create(IDB_EVENT_ICONS, 16, 1, RGB(255,0,255)));
	m_listCtrl.SetImageList(piml, LVSIL_SMALL);

	m_listCtrl.SetExtendedStyle(LVS_EX_FULLROWSELECT | m_listCtrl.GetExtendedStyle());

	HotspotVisitCollection visitedHotspots;
	CalculateHotspotVisits(m_positions, m_pHotspotGrid, &visitedHotspots);

	for (HotspotVisitCollection::const_iterator i = visitedHotspots.begin(); i != visitedHotspots.end(); ++i)
	{
		InsertListItem(i->id, i->entryTime, i->exitTime, i->imageIndex);
	}

	GetDlgItem(IDCANCEL)->SetFocus();
	return FALSE;
}

int CWalkHotspotsDialog::InsertListItem(int hotspotId, FILETIME entryTime, FILETIME exitTime, int iImage)
{
	CString strID;
	strID.Format("%d", hotspotId);

	CString strEntryTime = FormatFileTimeMs(entryTime, &m_tzi);
	CString strExitTime = FormatFileTimeMs(exitTime, &m_tzi);
	CString strElapsedTime = FormatElapsedFileTimeMs(entryTime, exitTime);

	LVITEM item;
	memset(&item, 0, sizeof(LVITEM));
	item.mask = LVIF_TEXT|LVIF_IMAGE;
	item.iImage = iImage;
	item.iItem = m_listCtrl.GetItemCount();
	item.pszText = const_cast<TCHAR *>(LPCTSTR(strID));

	int nItem = m_listCtrl.InsertItem(&item);

	item.iSubItem = 1;
	item.pszText = const_cast<TCHAR *>(LPCTSTR(strEntryTime));
	m_listCtrl.SetItem(&item);

	item.iSubItem = 2;
	item.pszText = const_cast<TCHAR *>(LPCTSTR(strExitTime));
	m_listCtrl.SetItem(&item);

	item.iSubItem = 3;
	item.pszText = const_cast<TCHAR *>(LPCTSTR(strElapsedTime));
	m_listCtrl.SetItem(&item);

	return nItem;
}

void CWalkHotspotsDialog::OnDestroy()
{
	delete m_listCtrl.GetImageList(LVSIL_SMALL);

	CDialog::OnDestroy();
}
