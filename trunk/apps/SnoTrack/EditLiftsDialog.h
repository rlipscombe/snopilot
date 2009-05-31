#pragma once
#include "resource.h"
#include "Lift.h"

class Projection;

// CEditLiftsDialog dialog
class CEditLiftsDialog : public CDialog
{
	CListCtrl m_listCtrl;
	LiftCollection m_lifts;
	Projection *m_proj;

	DECLARE_DYNAMIC(CEditLiftsDialog)

public:
	CEditLiftsDialog(const LiftCollection &lifts, Projection *proj, CWnd* pParent = NULL);   // standard constructor
	virtual ~CEditLiftsDialog();

	void GetLifts(LiftCollection *lifts);

	virtual BOOL OnInitDialog();

// Dialog Data
	enum { IDD = IDD_EDIT_LIFTS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	afx_msg void OnDestroy();
	afx_msg void OnBnClickedDelete();
	afx_msg void OnBnClickedEdit();
	afx_msg void OnBnClickedNew();
	afx_msg void OnNMDblclkListctrl(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnItemchangedListctrl(NMHDR *pNMHDR, LRESULT *pResult);

	DECLARE_MESSAGE_MAP()

private:
	void PopulateListCtrl();
	int InsertListItem(const Lift &lift);
	void UpdateItem(int nItem);
	void EnableControls();
};
