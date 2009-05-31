// ChildFrm.h : interface of the CChildFrame class
//

#pragma once
#include "SnoTrackSplitterWnd.h"

class CCartesianView;

class CChildFrame : public CMDIChildWnd
{
	CSnoTrackSplitterWnd m_wndHorizSplitter;
	CSnoTrackSplitterWnd m_wndVertSplitter;

	DECLARE_DYNCREATE(CChildFrame)

public:
	CChildFrame();
	void DeferReplaceView(CRuntimeClass *pNewViewClass);
	BOOL ReplaceView(CRuntimeClass *pNewViewClass);

	// Can return NULL if there isn't one.
	CCartesianView *GetCartesianView();

public:
	virtual void ActivateFrame(int nCmdShow = -1);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

protected:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	virtual void OnUpdateFrameMenu(BOOL bActive, CWnd* pActiveWnd, HMENU hMenuAlt);

// Implementation
public:
	virtual ~CChildFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	afx_msg void OnDestroy();
	afx_msg LRESULT OnReplaceView(WPARAM wParam, LPARAM lParam);
	afx_msg void OnSize(UINT nType, int cx, int cy);

	DECLARE_MESSAGE_MAP()
};
