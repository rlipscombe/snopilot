/* ColumnListCtrl.cpp
 *
 * Copyright (C) 2003 Roger Lipscombe, http://www.differentpla.net/~roger/
 */

#include "StdAfx.h"
#include "ColumnListCtrl.h"
#include "ListCtrlItem.h"
#include "column.h"
#include <algorithm>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CColumnListCtrl

CColumnListCtrl::CColumnListCtrl()
	: m_pSortColumn(NULL), m_bSortAscending(true)
{
}

CColumnListCtrl::~CColumnListCtrl()
{
}


BEGIN_MESSAGE_MAP(CColumnListCtrl, CListCtrl)
	ON_WM_DESTROY()
	ON_NOTIFY_REFLECT(LVN_GETDISPINFO, OnGetDispInfo)
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnColumnClick)
	ON_NOTIFY_REFLECT(LVN_DELETEALLITEMS, OnDeleteAllItems)
	ON_NOTIFY_REFLECT(LVN_DELETEITEM, OnDeleteItem)
END_MESSAGE_MAP()

const Column *CColumnListCtrl::GetColumnFromSubItem(int iSubItem)
{
	const Column *pColumn = m_columns[iSubItem];
	ASSERT(pColumn);
	return pColumn;
}

void CColumnListCtrl::InsertColumn(Column *pColumn)
{
	int nCol = (int)m_columns.size();
	tstring heading = pColumn->GetHeading();
	int format = pColumn->GetFormat();
	int width = pColumn->GetDefaultWidth();
	int sub_item = pColumn->GetSubItem();

	CListCtrl::InsertColumn(nCol, heading.c_str(), format, width, sub_item);
	m_columns[sub_item] = pColumn;
}

void CColumnListCtrl::SortBySubItem(int iSubItem, bool bSortAscending)
{
	m_pSortColumn = GetColumnFromSubItem(iSubItem);
	m_bSortAscending = bSortAscending;

	SortByColumn(m_pSortColumn, m_bSortAscending);
}

bool CColumnListCtrl::SortByColumnPredicate::operator() (const CListCtrlItem *lhs, const CListCtrlItem *rhs) const
{
	int result = m_pColumn->Compare(lhs, rhs);

	if (m_ascending)
		return result < 0;
	else
		return result >= 0;
}

void CColumnListCtrl::SortByColumn(const Column *pColumn, bool ascending)
{
	// If we want to keep the selection the same, we need to remember a unique ID for the
	// currently-selected items, and then re-select them later.
	// Similarly for the focused item.

	// We _ought_ to keep track of this stuff by handling LVN_ITEMCHANGED; for now, we'll do it here.
	StoreSelectedItems();

	std::stable_sort(m_list_entries.begin(), m_list_entries.end(), SortByColumnPredicate(pColumn, ascending));

	RestoreSelectedItems();
	RedrawVisibleItems();
}

/** Remember which items are selected and/or focused.  We need to keep this information across a sort. */
void CColumnListCtrl::StoreSelectedItems()
{
	for (int nItem = GetNextItem(-1, LVNI_ALL); nItem != -1; nItem = GetNextItem(nItem, LVNI_ALL))
	{
		const int nStateMask = LVIS_SELECTED|LVIS_FOCUSED;
		int nState = GetItemState(nItem, nStateMask);

		int new_flags = NONE;
		if (nState & LVIS_SELECTED)
			new_flags |= SELECTED;
		if (nState & LVIS_FOCUSED)
			new_flags |= FOCUSED;

		m_list_entries[nItem].second = new_flags;
	}
}

/** Restore the selected/focused items.  We kept this information across a sort. */
void CColumnListCtrl::RestoreSelectedItems()
{
	for (int nItem = GetNextItem(-1, LVNI_ALL); nItem != -1; nItem = GetNextItem(nItem, LVNI_ALL))
	{
		int nState = 0;
		int new_flags = m_list_entries[nItem].second;
		if (new_flags & SELECTED)
			nState |= LVIS_SELECTED;
		if (new_flags & FOCUSED)
			nState |= LVIS_FOCUSED;

		const int nStateMask = LVIS_SELECTED|LVIS_FOCUSED;
		SetItemState(nItem, nState, nStateMask);
	}
}

void CColumnListCtrl::RedrawVisibleItems()
{
	// By default, we'll force the list control to redraw everything.
	int nFirst = 0;
	int nLast = GetItemCount();

	// In report view, we can optimise this by
	// figuring out which items are visible.
	DWORD dwStyle = GetStyle();
	dwStyle &= LVS_TYPEMASK;
	if (dwStyle == LVS_REPORT)
	{
		nFirst = GetTopIndex();
		nLast = nFirst + GetCountPerPage();
	}

	// Redraw the relevant items.
	RedrawItems(nFirst, nLast);
}

void CColumnListCtrl::InsertItem(CListCtrlItem *pItem)
{
	list_entry_t list_entry = std::make_pair(pItem, NONE);
	m_list_entries.push_back(list_entry);

	// I hope that it's not too expensive to do this on every item added to the list.
	ASSERT(GetStyle() & LVS_OWNERDATA);
	SetItemCountEx((int)m_list_entries.size(), LVSICF_NOINVALIDATEALL|LVSICF_NOSCROLL);
}

const CListCtrlItem *CColumnListCtrl::GetItem(int iItem) const
{
	if (iItem >= 0 && iItem < (int)m_list_entries.size())
		return m_list_entries[iItem].first;

	return NULL;
}

/////////////////////////////////////////////////////////////////////////////
// CColumnListCtrl message handlers

void CColumnListCtrl::OnGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	int item_index = pDispInfo->item.iItem;
	ASSERT(item_index >= 0 && item_index < (int)m_list_entries.size());
	const list_entry_t &list_entry = m_list_entries[item_index];
	const CListCtrlItem *pItem = list_entry.first;

	if (pDispInfo->item.mask & LVIF_TEXT)
	{
		// We can copy the text into the pszText buffer (making sure that
		// we don't store more than cchTextMax characters).
		//
		// Alternatively, we can point pszText directly at our data.
		// The documentation says that if we do this, we should keep this buffer
		// around until either the item is deleted (we'll see LVN_DELETEITEM),
		// or we've received two additional LVN_GETDISPINFO messages.
		// 
		// This is a little arbitrary, so unless we can guarantee that the buffer
		// will live as long as the item, we'll copy the text in there.

		// The column knows how to get the data from the item, so we'll go there.
		int iSubItem = pDispInfo->item.iSubItem;
		const Column *pColumn = GetColumnFromSubItem(iSubItem);
		if (pColumn)
		{
			tstring text = pColumn->GetText(pItem);
			int cchTextMax = pDispInfo->item.cchTextMax;
			_tcsncpy(pDispInfo->item.pszText, text.c_str(), cchTextMax-1);
			pDispInfo->item.pszText[cchTextMax-1] = 0;
		}
	}

	if (pDispInfo->item.mask & LVIF_STATE)
	{
		pDispInfo->item.state = pItem->GetState();
		pDispInfo->item.stateMask = pItem->GetStateMask();
	}

	if (pDispInfo->item.mask & LVIF_IMAGE)
	{
		pDispInfo->item.iImage = pItem->GetImage();
	}

	*pResult = 0;
}

void CColumnListCtrl::OnDestroy() 
{
	for (list_entries_t::iterator i = m_list_entries.begin(); i != m_list_entries.end(); ++i)
	{
		CListCtrlItem *pItem = i->first;
		delete pItem;
	}

	for (columns_t::iterator i = m_columns.begin(); i != m_columns.end(); ++i)
	{
		Column *pColumn = i->second;
		delete pColumn;
	}

	CListCtrl::OnDestroy();
}

/** Windows XP sorting rules:
* If we're already sorting by that column, then reverse the direction.
* If we're not already sorting by that column, then it's always ascending.
*/
void CColumnListCtrl::OnColumnClick(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	CWaitCursor wait;

	// Based on which item was clicked on, sort our data array.
	int iSubItem = pNMListView->iSubItem;
	const Column *pColumn = GetColumnFromSubItem(iSubItem);

	// Are we already sorting by this column?
	if (GetSortColumn() == pColumn)
	{
		// Then we need to invert the order from last time.
		m_bSortAscending = !m_bSortAscending;
	}
	else
	{
		// Then we need to force the order to ascending, and remember
		// which column we're using.
		m_bSortAscending = true;
		m_pSortColumn = pColumn;
	}

	SortByColumn(pColumn, m_bSortAscending);
	*pResult = 0;
}

void CColumnListCtrl::OnDeleteAllItems(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

	for (list_entries_t::iterator i = m_list_entries.begin(); i != m_list_entries.end(); ++i)
	{
		CListCtrlItem *pItem = i->first;
		delete pItem;
	}

	m_list_entries.clear();
	SetItemCountEx((int)m_list_entries.size(), LVSICF_NOINVALIDATEALL|LVSICF_NOSCROLL);

	*pResult = 0;
}

void CColumnListCtrl::OnDeleteItem(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

	// Remove the item from the list, where relevant.
	list_entries_t::iterator i = m_list_entries.begin() + pNMLV->iItem;
	CListCtrlItem *pItem = i->first;
	delete pItem;
	m_list_entries.erase(i);

	SetItemCountEx((int)m_list_entries.size(), LVSICF_NOINVALIDATEALL|LVSICF_NOSCROLL);

	*pResult = 0;
}
