#pragma once
#include "resource.h"
#include "GridBounds.h"

// CDefineHotspotGridDialog dialog

class CDefineHotspotGridDialog : public CDialog
{
	GridBounds m_gridBounds;
	int m_gridSpacing;

	DECLARE_DYNAMIC(CDefineHotspotGridDialog)

public:
	CDefineHotspotGridDialog(const GridBounds &gridBounds, int gridSpacing, CWnd* pParent = NULL);   // standard constructor
	virtual ~CDefineHotspotGridDialog();

	virtual BOOL OnInitDialog();

	GridBounds GetGridBounds() const { return m_gridBounds; }
	int GetGridSpacing() const { return m_gridSpacing; }

// Dialog Data
	enum { IDD = IDD_DEFINE_HOTSPOT_GRID };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	void UpdateEditControls();
protected:
	virtual void OnOK();
};
