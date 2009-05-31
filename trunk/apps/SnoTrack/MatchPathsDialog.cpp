// MatchPathsDialog.cpp : implementation file
//

#include "stdafx.h"
#include "MatchPathsDialog.h"
#include "win32/TimeZoneInformation.h"
#include "HotspotPath.h"

// CMatchPathsDialog dialog
IMPLEMENT_DYNAMIC(CMatchPathsDialog, CDialog)

CMatchPathsDialog::CMatchPathsDialog(const TIME_ZONE_INFORMATION *pTimeZoneInformation, CWnd* pParent /*=NULL*/)
	: CDialog(CMatchPathsDialog::IDD, pParent), m_tzi(*pTimeZoneInformation)
{
}

CMatchPathsDialog::~CMatchPathsDialog()
{
}

void CMatchPathsDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LISTCTRL, m_listCtrl);
}

BEGIN_MESSAGE_MAP(CMatchPathsDialog, CDialog)
	ON_WM_SIZE()
	ON_NOTIFY(LVN_GETDISPINFO, IDC_LISTCTRL, OnLvnGetdispinfoListctrl)
	ON_NOTIFY(LVN_DELETEITEM, IDC_LISTCTRL, OnLvnDeleteitemListctrl)
	ON_WM_DESTROY()
END_MESSAGE_MAP()

// CMatchPathsDialog message handlers
void CMatchPathsDialog::AddMatchedPath(const HotspotPath &path, FILETIME pathEntryTime, FILETIME pathExitTime)
{
	AddEvent(ConvertUTF8ToSystem(path.GetName()).c_str(), pathEntryTime, pathExitTime, path.GetImageIndex());
}

void CMatchPathsDialog::AddUnmatchedHotspot(int id, FILETIME entryTime, FILETIME exitTime, int imageIndex)
{
	CString desc;
	desc.Format("Hotspot #%d", id);
	AddEvent(desc, entryTime, exitTime, imageIndex);
}

void CMatchPathsDialog::AddEvent(const CString &description, FILETIME startTime, FILETIME endTime, int imageIndex)
{
	MatchEvent *event = new MatchEvent;
	event->description = description;
	event->startTime = startTime;
	event->endTime = endTime;
	event->imageIndex = imageIndex;

	m_events.push_back(event);
}

/* static */
int CALLBACK CMatchPathsDialog::CompareStartTimeFunction(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	const MatchEvent *pItem1 = reinterpret_cast<const MatchEvent *>(lParam1);
	const MatchEvent *pItem2 = reinterpret_cast<const MatchEvent *>(lParam2);

	return CompareFileTime(&pItem1->startTime, &pItem2->startTime);
}

#define SUBITEM_DESCRIPTION 0
#define SUBITEM_START_TIME 1
#define SUBITEM_END_TIME 2

BOOL CMatchPathsDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	CRect rect;
	m_listCtrl.GetClientRect(&rect);

	int cx = rect.Width() - 20;
	m_listCtrl.InsertColumn(0, _T("Description"), LVCFMT_LEFT, cx / 3, SUBITEM_DESCRIPTION);
	m_listCtrl.InsertColumn(1, _T("Start Time"), LVCFMT_LEFT, cx / 3, SUBITEM_START_TIME);
	m_listCtrl.InsertColumn(2, _T("End Time"), LVCFMT_LEFT, cx / 3, SUBITEM_END_TIME);

	CImageList *iml = NEW CImageList;
	VERIFY(iml->Create(IDB_EVENT_ICONS, 16, 1, RGB(255,0,255)));

	m_listCtrl.SetImageList(iml, LVSIL_SMALL);

	for (EventCollection::const_iterator i = m_events.begin(); i != m_events.end(); ++i)
	{
		MatchEvent *event = *i;

		LVITEM item;
		memset(&item, 0, sizeof(LVITEM));
		item.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
		item.iItem = m_listCtrl.GetItemCount();
		item.pszText = LPSTR_TEXTCALLBACK;
		item.iImage = I_IMAGECALLBACK;
		item.lParam = reinterpret_cast<LPARAM>(event);

		int nItem = m_listCtrl.InsertItem(&item);
		ASSERT(nItem != -1);
	}

	m_listCtrl.SortItems(CompareStartTimeFunction, 0);

	m_listCtrl.SetFocus();
	return FALSE;
}

void CMatchPathsDialog::OnSize(UINT nType, int cx, int cy)
{
	if (m_listCtrl.GetSafeHwnd())
	{
		CRect clientRect;
		GetClientRect(&clientRect);

		CRect listRect(clientRect);
		CRect buttonRect(clientRect);

		listRect.InflateRect(-12, -12, -12, -12);
		buttonRect.InflateRect(-12, -12, -12, -12);

		CRect currentButtonRect;
		GetDlgItem(IDCANCEL)->GetWindowRect(&currentButtonRect);

		buttonRect.left = buttonRect.right - currentButtonRect.Width();
		buttonRect.bottom = buttonRect.top + currentButtonRect.Height();

		listRect.right = buttonRect.left - 12;

		m_listCtrl.SetWindowPos(NULL, listRect.left, listRect.top, listRect.Width(), listRect.Height(), SWP_NOZORDER);
		GetDlgItem(IDCANCEL)->SetWindowPos(NULL, buttonRect.left, buttonRect.top, buttonRect.Width(), buttonRect.Height(), SWP_NOZORDER);
	}

	CDialog::OnSize(nType, cx, cy);
}

void CMatchPathsDialog::OnLvnGetdispinfoListctrl(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLVDISPINFO *pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);

	const MatchEvent *pItem = reinterpret_cast<const MatchEvent *>(pDispInfo->item.lParam);

	UINT mask = pDispInfo->item.mask;
	if (mask & LVIF_TEXT)
	{
		CString strText;

		// This isn't actually the subitem:
		int nCol = pDispInfo->item.iSubItem;

		LVCOLUMN column;
		memset(&column, 0, sizeof(LVCOLUMN));
		column.mask = LVCF_SUBITEM;
		VERIFY(m_listCtrl.GetColumn(nCol, &column));
		int iSubItem = column.iSubItem;

		switch (iSubItem)
		{
		case SUBITEM_DESCRIPTION:
			strText = pItem->description;
			break;

		case SUBITEM_START_TIME:
			strText = FormatUtcFileTimeInTzSpecificLocalTime(pItem->startTime, &m_tzi);
			break;

		case SUBITEM_END_TIME:
			strText = FormatUtcFileTimeInTzSpecificLocalTime(pItem->endTime, &m_tzi);
			break;
		}

		// Make sure that we don't overflow the buffer.
		_tcsncpy(pDispInfo->item.pszText, strText, pDispInfo->item.cchTextMax);

		// Because _tcsncpy won't put the NULL terminator in the buffer if we're right
		// at the end, we need to do it ourselves:
		pDispInfo->item.pszText[pDispInfo->item.cchTextMax-1] = _T('\0');
	}

	if (mask & LVIF_IMAGE)
	{
		pDispInfo->item.iImage = pItem->imageIndex;
	}

	*pResult = 0;
}

void CMatchPathsDialog::OnLvnDeleteitemListctrl(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

	MatchEvent *event = reinterpret_cast<MatchEvent *>(pNMLV->lParam);
	delete event;

	*pResult = 0;
}

void CMatchPathsDialog::OnDestroy()
{
	delete m_listCtrl.GetImageList(LVSIL_SMALL);

	CDialog::OnDestroy();
}
