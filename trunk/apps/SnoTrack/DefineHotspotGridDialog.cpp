// DefineHotspotGridDialog.cpp : implementation file
//

#include "stdafx.h"
#include "DefineHotspotGridDialog.h"
#include "LatLon.h"

// CDefineHotspotGridDialog dialog

IMPLEMENT_DYNAMIC(CDefineHotspotGridDialog, CDialog)

CDefineHotspotGridDialog::CDefineHotspotGridDialog(const GridBounds &gridBounds, int gridSpacing, CWnd* pParent /*=NULL*/)
	: CDialog(CDefineHotspotGridDialog::IDD, pParent), m_gridBounds(gridBounds), m_gridSpacing(gridSpacing)
{
}

CDefineHotspotGridDialog::~CDefineHotspotGridDialog()
{
}

void CDefineHotspotGridDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CDefineHotspotGridDialog, CDialog)
END_MESSAGE_MAP()


// CDefineHotspotGridDialog message handlers

BOOL CDefineHotspotGridDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	UpdateEditControls();

	return TRUE;
}

void CDefineHotspotGridDialog::UpdateEditControls()
{
	SetDlgItemText(IDC_MIN_LON, FormatLongitude(m_gridBounds.m_minLongitude, FORMAT_LON_COLONS));
	SetDlgItemText(IDC_MAX_LON, FormatLongitude(m_gridBounds.m_maxLongitude, FORMAT_LON_COLONS));
	SetDlgItemText(IDC_MIN_LAT, FormatLatitude(m_gridBounds.m_minLatitude, FORMAT_LAT_COLONS));
	SetDlgItemText(IDC_MAX_LAT, FormatLatitude(m_gridBounds.m_maxLatitude, FORMAT_LAT_COLONS));

	SetDlgItemInt(IDC_GRID_SPACING, m_gridSpacing, FALSE);
}

void CDefineHotspotGridDialog::OnOK()
{
	struct ParseThing
	{
		UINT nIDC;
		bool (*fn) (const char *, double *);
		double *pValue;
		const char *message;
	};
	ParseThing things[] = {
		{ IDC_MIN_LON, ParseLongitude, &m_gridBounds.m_minLongitude, "Failed to parse minimum longitude", },
		{ IDC_MAX_LON, ParseLongitude, &m_gridBounds.m_maxLongitude, "Failed to parse maximum longitude", },
		{ IDC_MIN_LAT, ParseLatitude, &m_gridBounds.m_minLatitude, "Failed to parse minimum latitude", },
		{ IDC_MAX_LAT, ParseLatitude, &m_gridBounds.m_maxLatitude, "Failed to parse maximum latitude", },
		{ -1, NULL, NULL, NULL, },
	};

	for (const ParseThing *p = things; p->nIDC != -1; ++p)
	{
		CString str;
		GetDlgItemText(p->nIDC, str);

		if (!(p->fn)(str, p->pValue))
		{
			AfxMessageBox(p->message);
			GetDlgItem(p->nIDC)->SetFocus();
			static_cast<CEdit *>(GetDlgItem(p->nIDC))->SetSel(0, -1);
			return;
		}
	}

	if ((m_gridBounds.m_minLatitude >= m_gridBounds.m_maxLatitude) ||
		(m_gridBounds.m_minLongitude >= m_gridBounds.m_maxLongitude))
	{
		AfxMessageBox("min/max are the wrong way round");
		return;
	}

	m_gridSpacing = GetDlgItemInt(IDC_GRID_SPACING, NULL, FALSE);

	CDialog::OnOK();
}
