// EditPathDialog.cpp : implementation file
//

#include "stdafx.h"
#include "EditPathDialog.h"

// CEditPathDialog dialog

IMPLEMENT_DYNAMIC(CEditPathDialog, CDialog)

CEditPathDialog::CEditPathDialog(const HotspotPath &path, CWnd* pParent /*=NULL*/)
	: CDialog(CEditPathDialog::IDD, pParent), m_path(path)
{
}

CEditPathDialog::~CEditPathDialog()
{
}

void CEditPathDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LISTCTRL, m_listCtrl);
}


BEGIN_MESSAGE_MAP(CEditPathDialog, CDialog)
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CEditPathDialog message handlers

BOOL CEditPathDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	CImageList *iml = new CImageList;
	VERIFY(iml->Create(IDB_EVENT_ICONS, 16, 1, RGB(255,0,255)));
	m_listCtrl.SetImageList(iml, LVSIL_SMALL);

	for (int i = 0; i < iml->GetImageCount(); ++i)
	{
		CString str;
		str.Format("%d", i);
		m_listCtrl.InsertItem(LVIF_IMAGE|LVIF_TEXT, i, str, 0, 0, i, 0);
	}

	SetDlgItemText(IDC_PATH_NAME, ConvertUTF8ToSystem(m_path.GetName()).c_str());
	int imageIndex = m_path.GetImageIndex();
	m_listCtrl.SetItemState(imageIndex, LVIS_SELECTED, LVIS_SELECTED);

	CString strHotspotPath;
	for (HotspotPath::const_iterator i = m_path.begin(); i != m_path.end(); ++i)
	{
		CString str;
		str.Format("%d", *i);

		if (!strHotspotPath.IsEmpty())
			strHotspotPath += ", ";
		strHotspotPath += str;
	}

	SetDlgItemText(IDC_PATH_HOTSPOTS, strHotspotPath);

	// TODO: Generalise this a bit more, so that paths can belong to a category?
	// TODO: Make it so that the category controls the icon, and adds a colour?
	CheckDlgButton(IDC_PATH_IS_LIFT, m_path.IsLift());

	GetDlgItem(IDC_PATH_NAME)->SetFocus();
	static_cast<CEdit *>(GetDlgItem(IDC_PATH_NAME))->SetSel(0, -1);
	return FALSE;
}

void CEditPathDialog::OnOK()
{
	m_path.clear();

	CString str;
	GetDlgItemText(IDC_PATH_HOTSPOTS, str);

	BOOL bOK = TRUE;
	int pos0 = 0;
	for (;;)
	{
		CString token;

		int pos1 = str.Find(',', pos0);
		if (pos1 != -1)
			token = str.Mid(pos0, pos1-pos0);
		else
			token = str.Mid(pos0);
		
		// Attempt to parse that as a number.
		if (token.IsEmpty())
		{
			TRACE("token at %d is empty, therefore not a number!\n", pos0);

			CString message("Empty token");
			AfxMessageBox(message, MB_OK | MB_ICONEXCLAMATION);
			static_cast<CEdit *>(GetDlgItem(IDC_PATH_HOTSPOTS))->SetSel(pos0, pos1);
			GetDlgItem(IDC_PATH_HOTSPOTS)->SetFocus();
			bOK = FALSE;

			break;
		}
		else
		{
			char *p;
			int n = strtol(token, &p, 10);
			if (p && *p)
			{
				TRACE("'%s' (at %d) is not a number!\n", LPCTSTR(token), pos0);

				CString message;
				message.Format("'%s' is not a number!", LPCTSTR(token));
				AfxMessageBox(message, MB_OK | MB_ICONEXCLAMATION);
				static_cast<CEdit *>(GetDlgItem(IDC_PATH_HOTSPOTS))->SetSel(pos0, pos1);
				GetDlgItem(IDC_PATH_HOTSPOTS)->SetFocus();
				bOK = FALSE;

				break;
			}
			else
			{
				TRACE("'%s' is a number: %d\n", LPCTSTR(token), n);

				// TODO: Verify that it's actually a valid hotspot ID.

				m_path.push_back(n);
			}
		}

		if (pos1 == -1)
			break;

		pos0 = pos1+1;
	}

	if (bOK)
	{
		CString pathName;
		GetDlgItemText(IDC_PATH_NAME, pathName);

		int imageIndex = m_listCtrl.GetNextItem(-1, LVNI_SELECTED);

		m_path.SetName(ConvertSystemToUTF8(pathName));
		m_path.SetImageIndex(imageIndex);
		m_path.SetLift(IsDlgButtonChecked(IDC_PATH_IS_LIFT) != 0);
		CDialog::OnOK();
	}
}

void CEditPathDialog::OnDestroy()
{
	delete m_listCtrl.GetImageList(LVSIL_SMALL);

	CDialog::OnDestroy();
}
