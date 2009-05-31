// EditLiftDialog.cpp : implementation file
//

#include "stdafx.h"
#include "EditLiftDialog.h"
#include "LatLon.h"
#include "Projection.h"

// CEditLiftDialog dialog

IMPLEMENT_DYNAMIC(CEditLiftDialog, CDialog)

CEditLiftDialog::CEditLiftDialog(const Lift &lift, Projection *proj, CWnd* pParent /*=NULL*/)
	: CDialog(CEditLiftDialog::IDD, pParent), m_lift(lift), m_proj(proj)
{
}

CEditLiftDialog::~CEditLiftDialog()
{
}

void CEditLiftDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CEditLiftDialog, CDialog)
END_MESSAGE_MAP()


// CEditLiftDialog message handlers

BOOL CEditLiftDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetDlgItemText(IDC_LIFT_NAME, ConvertUTF8ToSystem(m_lift.GetLabel()).c_str());

	SetDlgItemText(IDC_TOP_LON, FormatLongitude(m_lift.GetTopLongitude(), FORMAT_LON_COLONS));
	SetDlgItemText(IDC_TOP_LAT, FormatLatitude(m_lift.GetTopLatitude(), FORMAT_LAT_COLONS));

	SetDlgItemText(IDC_BOT_LON, FormatLongitude(m_lift.GetBottomLongitude(), FORMAT_LON_COLONS));
	SetDlgItemText(IDC_BOT_LAT, FormatLatitude(m_lift.GetBottomLatitude(), FORMAT_LAT_COLONS));

	GetDlgItem(IDC_LIFT_NAME)->SetFocus();
	return FALSE;
}

void CEditLiftDialog::OnOK()
{
	double bot_lon, bot_lat;
	double top_lon, top_lat;

	struct ParseThing
	{
		UINT nIDC;
		bool (*fn) (const char *, double *);
		double *pValue;
		const char *message;
	};
	ParseThing things[] = {
		{ IDC_TOP_LON, ParseLongitude, &top_lon, "Failed to parse top longitude", },
		{ IDC_TOP_LAT, ParseLatitude, &top_lat, "Failed to parse top latitude", },
		{ IDC_BOT_LON, ParseLongitude, &bot_lon, "Failed to parse bottom longitude", },
		{ IDC_BOT_LAT, ParseLatitude, &bot_lat, "Failed to parse bottom latitude", },
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
	GetDlgItemText(IDC_LIFT_NAME, str);
	utf8_string label = ConvertSystemToUTF8(str);

	CPoint bot = m_proj->ForwardDeg(bot_lon, bot_lat);
	CPoint top = m_proj->ForwardDeg(top_lon, top_lat);

	m_lift = Lift(label, bot_lon, bot_lat, bot.x, bot.y, top_lon, top_lat, top.x, top.y);

	CDialog::OnOK();
}
