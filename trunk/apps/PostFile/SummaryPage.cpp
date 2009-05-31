// SummaryPage.cpp : implementation file
//

#include "stdafx.h"
#include "SummaryPage.h"
#include "PostFileSettings.h"

// CSummaryPage dialog

IMPLEMENT_DYNAMIC(CSummaryPage, CWizardPage)

CSummaryPage::CSummaryPage(PostFileSettings *pSettings)
	: CWizardPage(CSummaryPage::IDD, IDS_SUMMARY_PAGE_CAPTION, IDS_SUMMARY_PAGE_TITLE, IDS_SUMMARY_PAGE_SUBTITLE),
		m_pSettings(pSettings)
{
}

CSummaryPage::~CSummaryPage()
{
}

void CSummaryPage::DoDataExchange(CDataExchange* pDX)
{
	CWizardPage::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_SUMMARY, m_summaryCtrl);
}

#define WM_DEFERRED_INIT (WM_APP+22)

BEGIN_MESSAGE_MAP(CSummaryPage, CWizardPage)
	ON_MESSAGE(WM_DEFERRED_INIT, OnDeferredInit)
END_MESSAGE_MAP()


// CSummaryPage message handlers
CString EscapeBackslashes(CString s)
{
	s.Replace(_T("\\"), _T("\\\\"));
	return s;
}

BOOL CSummaryPage::OnSetActive()
{
	if (!CWizardPage::OnSetActive())
		return FALSE;

	SetWizardButtons(PSWIZB_BACK|PSWIZB_NEXT);
	EnableCancelButton(TRUE);

	InitialiseSummaryText();

	PostMessage(WM_DEFERRED_INIT);

	return TRUE;
}

void CSummaryPage::InitialiseSummaryText()
{
	// Load the RTF text:
	HINSTANCE hinst = AfxFindResourceHandle(MAKEINTRESOURCE(IDR_SUMMARY), _T("RTF"));
	HRSRC hrsrc = FindResource(hinst, MAKEINTRESOURCE(IDR_SUMMARY), _T("RTF"));
	unsigned cb = SizeofResource(hinst, hrsrc);
	HGLOBAL hglb = LoadResource(hinst, hrsrc);
	char *pData = (char *)LockResource(hglb);

	CString summaryText(pData, cb);

	CString strEmailAddress;
	CString strForumUsername;
	if (m_pSettings->m_bAnonymous)
	{
		strEmailAddress = "(Anonymous)";
		strForumUsername = "(Anonymous)";
	}
	else
	{
		strEmailAddress = m_pSettings->m_strEmailAddress;
		strForumUsername = m_pSettings->m_strForumUserName;
	}

	summaryText.Replace(_T("%email_address"), EscapeBackslashes(strEmailAddress));
	summaryText.Replace(_T("%forum_username"), EscapeBackslashes(strForumUsername));

	Product_Data_Type productData;
	m_pSettings->m_summary.GetProductData(&productData);
	CString productDescription = ConvertASCIIToSystem(productData.product_description).c_str();
	summaryText.Replace(_T("%gps_unit"), EscapeBackslashes(productDescription));

	CString strLocalFilename;
	PathCompactPathEx(strLocalFilename.GetBuffer(MAX_PATH), m_pSettings->m_strLocalFilename, 35, 0);
	strLocalFilename.ReleaseBuffer();
	
	summaryText.Replace(_T("%local_filename"), EscapeBackslashes(strLocalFilename));

	utf8_string comments = m_pSettings->m_summary.GetComments();
	summaryText.Replace(_T("%comments"), EscapeBackslashes(ConvertUTF8ToSystem(comments).c_str()));

	CString other;
	VERIFY(other.LoadString(IDS_OTHER_RESORT_FILENAME));

	tstring resortFilename = m_pSettings->m_summary.GetResortFileName();
	if (resortFilename.empty())
		resortFilename = other;
	summaryText.Replace(_T("%resort_filename"), EscapeBackslashes(resortFilename.c_str()));

#if 0
	utf8_string resortName;
	if (!resortFilename.empty())
	{
		ResortProperties resort;
		resort.LoadFromFile(resortFilename.c_str());

		resortName = resort.m_resortName;
	}
	else
	{
		resortName = m_pSettings->m_summary.GetCustomResortName();
	}

	summaryText.Replace(_T("%resort_name"), EscapeBackslashes(ConvertUTF8ToSystem(resortName).c_str()));
#endif

	summaryText.Replace(_T("%tracklog_size"), EscapeBackslashes(m_pSettings->m_summary.GetTracklogSizeStr()));
	summaryText.Replace(_T("%waypoint_count"), EscapeBackslashes(m_pSettings->m_summary.GetWaypointCountStr()));

	m_summaryCtrl.SetWindowText(summaryText);
	return;
}

LRESULT CSummaryPage::OnDeferredInit(WPARAM wParam, LPARAM lParam)
{
	m_summaryCtrl.SetSel(0, 0);
	
	CWnd *pNextButton = static_cast<CPropertySheet *>(GetParent())->GetDlgItem(0x3024);
	if (pNextButton)
		pNextButton->SetFocus();

	return 0;
}
