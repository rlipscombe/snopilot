#pragma once
#include "resource.h"
#include "MatchedPathListBox.h"

// CSidebarView form view

class CSidebarView : public CFormView
{
	CMatchedPathListBox m_listBox;

	DECLARE_DYNCREATE(CSidebarView)

protected:
	CSidebarView();           // protected constructor used by dynamic creation
	virtual ~CSidebarView();

	virtual void OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/);
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

public:
	enum { IDD = IDD_SIDEBARVIEW };
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	virtual void OnInitialUpdate();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnLbnSelchangeListbox();
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
};


