/* ColumnListCtrl.h
 *
 * Copyright (C) 2003 Roger Lipscombe, http://www.differentpla.net/~roger/
 */

#if !defined(AFX_COLUMNLISTCTRL_H__D5864E3D_ACA1_4FB0_BD77_B43A8565E2F3__INCLUDED_)
#define AFX_COLUMNLISTCTRL_H__D5864E3D_ACA1_4FB0_BD77_B43A8565E2F3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <vector>
#include <map>

class Column;
class CListCtrlItem;

/** An extension to CListCtrl that supports owner-data, and has columns.
 */
class CColumnListCtrl : public CListCtrl
{
	enum flags_t {
		NONE = 0x00,
		SELECTED = 0x01,
		FOCUSED = 0x02,
	};

	typedef std::pair< CListCtrlItem *, int > list_entry_t;
	typedef std::vector< list_entry_t > list_entries_t;
	list_entries_t m_list_entries;

	typedef std::map< int, Column * > columns_t;
	columns_t m_columns;

	const Column *m_pSortColumn;
	bool m_bSortAscending;

	class SortByColumnPredicate
	{
		const Column *m_pColumn;
		int m_ascending;

	public:
		SortByColumnPredicate(const Column *pColumn, bool ascending)
			: m_pColumn(pColumn), m_ascending(ascending)
		{
		}

		bool operator() (const list_entry_t &lhs, const list_entry_t &rhs) const
		{
			return operator() (lhs.first, rhs.first);
		}

		bool operator() (const CListCtrlItem *lhs, const CListCtrlItem *rhs) const;
	};

public:
	CColumnListCtrl();
	virtual ~CColumnListCtrl();

	//{{AFX_VIRTUAL(CColumnListCtrl)
	//}}AFX_VIRTUAL

public:
	void InsertColumn(Column *pColumn);
	void SortBySubItem(int iSubItem, bool bSortAscending);

public:
	void InsertItem(CListCtrlItem *pItem);
	const CListCtrlItem *GetItem(int iItem) const;

protected:
	/** @todo Move this into a ColumnSet class. */
	const Column *GetColumnFromSubItem(int iSubItem);
	const Column *GetSortColumn() const { return m_pSortColumn; }

	void SortByColumn(const Column *pColumn, bool ascending);

private:
	// When re-sorting the list control, we need to save and restore the focus and selection.
	void StoreSelectedItems();
	void RestoreSelectedItems();
	void RedrawVisibleItems();

protected:
	//{{AFX_MSG(CColumnListCtrl)
	afx_msg void OnGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDestroy();
	afx_msg void OnColumnClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeleteAllItems(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDeleteItem(NMHDR *pNMHDR, LRESULT *pResult);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COLUMNLISTCTRL_H__D5864E3D_ACA1_4FB0_BD77_B43A8565E2F3__INCLUDED_)
