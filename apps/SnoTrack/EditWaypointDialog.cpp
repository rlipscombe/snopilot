// EditWaypointDialog.cpp : implementation file
//

#include "stdafx.h"
#include "EditWaypointDialog.h"
#include "LatLon.h"
#include "Projection.h"

// CEditWaypointDialog dialog

IMPLEMENT_DYNAMIC(CEditWaypointDialog, CDialog)

CEditWaypointDialog::CEditWaypointDialog(const Waypoint &waypoint, Projection *proj, CWnd* pParent /*=NULL*/)
	: CDialog(CEditWaypointDialog::IDD, pParent), m_waypoint(waypoint), m_proj(proj)
{
}

CEditWaypointDialog::~CEditWaypointDialog()
{
}

void CEditWaypointDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CEditWaypointDialog, CDialog)
END_MESSAGE_MAP()

// CEditWaypointDialog message handlers
BOOL CEditWaypointDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetDlgItemText(IDC_WAYPOINT_NAME, ConvertUTF8ToSystem(m_waypoint.GetLabel()).c_str());
	SetDlgItemText(IDC_WAYPOINT_LON, FormatLongitude(m_waypoint.GetLongitude(), FORMAT_LON_COLONS));
	SetDlgItemText(IDC_WAYPOINT_LAT, FormatLatitude(m_waypoint.GetLongitude(), FORMAT_LAT_COLONS));

	return TRUE;
}

void CEditWaypointDialog::OnOK()
{
	double lon, lat;

	struct ParseThing
	{
		UINT nIDC;
		bool (*fn) (const char *, double *);
		double *pValue;
		const char *message;
	};
	ParseThing things[] = {
		{ IDC_WAYPOINT_LON, ParseLongitude, &lon, "Failed to parse longitude", },
		{ IDC_WAYPOINT_LAT, ParseLatitude, &lat, "Failed to parse latitude", },
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

	CString str;
	GetDlgItemText(IDC_WAYPOINT_NAME, str);
	utf8_string label = ConvertSystemToUTF8(str);

	CPoint pj = m_proj->ForwardDeg(lon, lat);

	m_waypoint = Waypoint(label, lon, lat, pj.x, pj.y);

	CDialog::OnOK();
}
