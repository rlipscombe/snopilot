// NewFileWizard.cpp : implementation file
//

#include "stdafx.h"
#include "NewFileWizard.h"

#include "NewFileWelcomePage.h"
#include "NewFileResortPage.h"
#include "NewFileCompletePage.h"

#include "resource.h"

#include "win32/VersionInfo.h"

// CNewFileWizard

IMPLEMENT_DYNAMIC(CNewFileWizard, CPropertySheet)

CNewFileWizard::CNewFileWizard(NewFileSettings *pSettings, UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage, HBITMAP hbmWatermark, HPALETTE hpalWatermark, HBITMAP hbmHeader)
	: CPropertySheet(nIDCaption, pParentWnd, iSelectPage, hbmWatermark, hpalWatermark, hbmHeader)
{
	AddPage(NEW CNewFileWelcomePage);
	AddPage(NEW CNewFileResortPage(pSettings));
	AddPage(NEW CNewFileCompletePage(pSettings));

	SetWizardMode();

	if (SupportsWizard97())
		m_psh.dwFlags |= PSH_WIZARD97;
}

CNewFileWizard::~CNewFileWizard()
{
	for (int i = 0; i < GetPageCount(); ++i)
	{
		CPropertyPage *pPage = GetPage(i);

		delete pPage;
	}
}

/* static */
INT_PTR CNewFileWizard::DoWizard(NewFileSettings *pSettings)
{
	HBITMAP hbmWatermark = ::LoadBitmap(AfxGetResourceHandle(), MAKEINTRESOURCE(IDB_NEW_FILE_WATERMARK));
    HPALETTE hpalWatermark = NULL;
    HBITMAP hbmHeader = ::LoadBitmap(AfxGetResourceHandle(), MAKEINTRESOURCE(IDB_NEW_FILE_HEADER));

	CNewFileWizard wizard(pSettings, IDS_NEW_FILE_WIZARD_CAPTION, NULL, 0, hbmWatermark, hpalWatermark, hbmHeader);
	INT_PTR nResult = wizard.DoModal();
	
	return nResult;
}

BEGIN_MESSAGE_MAP(CNewFileWizard, CPropertySheet)
END_MESSAGE_MAP()

// CNewFileWizard message handlers
