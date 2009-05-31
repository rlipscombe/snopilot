#include "StdAfx.h"
#include "AboutDlg.h"
#include "win32/VersionInfo.h"

CAboutDlg::CAboutDlg()
	: CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_WEBSITE_URL, m_websiteUrl);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

BOOL CAboutDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CVersionInfo ver;
	VERIFY(ver.Create());

	CString companyName = ver.GetString("CompanyName");
	CString productName = ver.GetString("ProductName");
	CString productVersion = ver.GetString("ProductVersion");
	CString version;
	version.Format(_T("%s Version %s"), (LPCTSTR)productName, (LPCTSTR)productVersion);

	CString version_suffix;

#if defined(_DEBUG)
	if (!version_suffix.IsEmpty())
		version_suffix += _T(", ");
	version_suffix += _T("Debug");
#endif

#if defined(UNICODE)
	if (!version_suffix.IsEmpty())
		version_suffix += _T(", ");
	version_suffix += _T("Unicode");
#endif

#if defined(ENABLE_DESIGN_MODE)
	if (!version_suffix.IsEmpty())
		version_suffix += _T(", ");
	version_suffix += _T("Internal");
#endif

	if (!version_suffix.IsEmpty())
		version += CString(" (") + version_suffix + CString(")");

	SetDlgItemText(IDC_VERSION, version);

	CString legalCopyright = ver.GetString("LegalCopyright");
	SetDlgItemText(IDC_COPYRIGHT, legalCopyright);

	return TRUE;
}
