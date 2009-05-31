#pragma once
#include "resource.h"
#include "win32_ui/FlatButton.h"
#include "win32_ui/ColumnListCtrl.h"
#include "win32_ui/column.h"
#include "win32_ui/ListCtrlItem.h"

// CStartPageView form view

class CStartPageView : public CFormView
{
	CFont m_boldFont;
	CRecentFileList *m_pRecentFileList;

	DECLARE_DYNCREATE(CStartPageView)

protected:
	CStartPageView();           // protected constructor used by dynamic creation
	virtual ~CStartPageView();

public:
	enum { IDD = IDD_STARTPAGEVIEW };
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	CColumnListCtrl m_listCtrl;
	CFlatButton m_newFileButton;
	CFlatButton m_openFileButton;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnDraw(CDC* /*pDC*/);
	virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);

protected:
	DECLARE_MESSAGE_MAP()

	afx_msg void OnUpdateFileSave(CCmdUI *pCmdUI);
	afx_msg void OnUpdateFileSaveAs(CCmdUI *pCmdUI);
	afx_msg void OnUpdateWindowNew(CCmdUI *pCmdUI);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBnClickedNewFile();
	afx_msg void OnBnClickedOpenFile();
	virtual void OnInitialUpdate();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnLvnItemActivateListctrl(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnGetInfoTipListctrl(NMHDR *pNMHDR, LRESULT *pResult);

private:
	void PopulateListCtrl();
	void RecalcLayout();
};

class NameColumn : public Column
{
public:
	virtual tstring GetHeading() const { return _T("Name"); }
	virtual int GetFormat() const { return LVCFMT_LEFT; }
	virtual int GetDefaultWidth() const { return 240; }
	virtual int GetSubItem() const { return 0; }
    virtual tstring GetText(const CListCtrlItem *pItem) const;
    virtual int Compare(const CListCtrlItem *lhs, const CListCtrlItem *rhs) const;
};

class CStartPageItem : public CListCtrlItem
{
	CString m_strFileName;
	CString m_strFullPath;

public:
	CStartPageItem(const TCHAR *pszFileName, const TCHAR *pszFullPath)
		: m_strFileName(pszFileName), m_strFullPath(pszFullPath)
	{
	}

	tstring GetName() const { return (LPCTSTR)m_strFileName; }
	tstring GetFullPath() const { return (LPCTSTR)m_strFullPath; }
};
