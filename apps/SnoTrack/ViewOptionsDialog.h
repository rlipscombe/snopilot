#pragma once

#include "layer.h"
#include "resource.h"

// CViewOptionsDialog dialog

class CViewOptionsDialog : public CDialog
{
	Layers *m_layers;
	CListCtrl m_listCtrl;

	DECLARE_DYNAMIC(CViewOptionsDialog)

public:
	CViewOptionsDialog(Layers *layers, CWnd* pParent = NULL);   // standard constructor
	virtual ~CViewOptionsDialog();

	virtual BOOL OnInitDialog();

// Dialog Data
	enum { IDD = IDD_VIEW_OPTIONS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	virtual void OnOK();
};
