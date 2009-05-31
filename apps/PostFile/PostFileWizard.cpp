// PostFileWizard.cpp : implementation file
//

#include "stdafx.h"
#include "PostFileWizard.h"
#include "PostFileSettings.h"
#include "win32/VersionInfo.h"

#include "WelcomePage.h"
#include "AgreementPage.h"
#include "ContactPage.h"
#include "FilenamePage.h"
#include "SummaryPage.h"
#include "ProgressPage.h"
#include "CompletePage.h"

#include "win32/ApplicationProfile.h"

// CPostFileWizard

IMPLEMENT_DYNAMIC(CPostFileWizard, CPropertySheet)

CPostFileWizard::CPostFileWizard(PostFileSettings *pSettings, PostFileResults *pResults,
								 UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage, HBITMAP hbmWatermark, HPALETTE hpalWatermark, HBITMAP hbmHeader)
	: CPropertySheet(nIDCaption, pParentWnd, iSelectPage, hbmWatermark, hpalWatermark, hbmHeader)
{
	m_hIcon = LoadIcon(AfxGetResourceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME));

	AddPage(new CWelcomePage);
	AddPage(new CAgreementPage(pSettings));
	AddPage(new CContactPage(pSettings));
	AddPage(new CFilenamePage(pSettings));
	AddPage(new CSummaryPage(pSettings));
	AddPage(new CProgressPage(pSettings, pResults));
	AddPage(new CCompletePage(pSettings, pResults));

	SetWizardMode();

	if (SupportsWizard97())
		m_psh.dwFlags |= PSH_WIZARD97;
}

CPostFileWizard::~CPostFileWizard()
{
	for (int i = 0; i < GetPageCount(); ++i)
	{
		CPropertyPage *pPage = GetPage(i);

		delete pPage;
	}
}

/* static */
INT_PTR CPostFileWizard::DoWizard(LPCTSTR lpszLocalFile, CWnd **ppWnd)
{
	PostFileSettings settings;
	PostFileResults results;

	AfxApplicationProfile appProfile(AfxGetApp());

	// Load the settings and results from the registry.  Loading the results might seem a
	// little strange, but our cached transfer speed goes in there.
	settings.Load(&appProfile);
	results.Load(&appProfile);

	// If the user specified a file on the command line, override the value in the registry.
	if (lpszLocalFile && *lpszLocalFile)
		settings.m_strLocalFilename = lpszLocalFile;

	settings.m_strServer = "powder.snopilot.com";
	settings.m_strObjectName = "/cgi-bin/upload2";

	HBITMAP hbmWatermark = ::LoadBitmap(AfxGetResourceHandle(), MAKEINTRESOURCE(IDB_WATERMARK));
	ASSERT(hbmWatermark);
    HPALETTE hpalWatermark = NULL;
    HBITMAP hbmHeader = ::LoadBitmap(AfxGetResourceHandle(), MAKEINTRESOURCE(IDB_HEADER));
	ASSERT(hbmHeader);

	CPostFileWizard dlg(&settings, &results, IDS_PROGRESS_WIZARD_CAPTION, NULL, 0, hbmWatermark, hpalWatermark, hbmHeader);
	*ppWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();

	settings.Save(&appProfile);
	results.Save(&appProfile);

	return nResponse;
}

BEGIN_MESSAGE_MAP(CPostFileWizard, CPropertySheet)
END_MESSAGE_MAP()

// CPostFileWizard message handlers

void CPostFileWizard::PreSubclassWindow()
{
//	ModifyStyle(0, WS_MINIMIZEBOX);

	CPropertySheet::PreSubclassWindow();
}

BOOL CPostFileWizard::OnInitDialog()
{
	BOOL bResult = CPropertySheet::OnInitDialog();

	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);

	return bResult;
}
