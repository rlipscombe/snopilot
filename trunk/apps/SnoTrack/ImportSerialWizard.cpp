// ImportSerialWizard.cpp : implementation file
//

#include "stdafx.h"
#include "ImportSerialWizard.h"

#include "ImportSerialWelcomePage.h"
#include "ImportSerialDevicePage.h"
#include "ImportSerialProgressPage.h"
#include "ImportSerialCompletePage.h"

#include "ImportSerialSettings.h"

#include "win32/VersionInfo.h"

// CImportSerialWizard

IMPLEMENT_DYNAMIC(CImportSerialWizard, CPropertySheet)

CImportSerialWizard::CImportSerialWizard(ImportSerialSettings *pSettings, ImportSerialResults *pResults, UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage, HBITMAP hbmWatermark, HPALETTE hpalWatermark, HBITMAP hbmHeader)
    : CPropertySheet(nIDCaption, pParentWnd, iSelectPage, hbmWatermark, hpalWatermark, hbmHeader)
{
    AddPage(NEW CImportSerialWelcomePage);
    AddPage(NEW CImportSerialDevicePage(pSettings));
    AddPage(NEW CImportSerialProgressPage(pSettings, pResults));
    AddPage(NEW CImportSerialCompletePage(pResults));

	SetWizardMode();

	if (SupportsWizard97())
		m_psh.dwFlags |= PSH_WIZARD97;
}

CImportSerialWizard::~CImportSerialWizard()
{
	for (int i = 0; i < GetPageCount(); ++i)
	{
		CPropertyPage *pPage = GetPage(i);

		delete pPage;
	}
}

INT_PTR CImportSerialWizard::DoWizard(ImportSerialSettings *pSettings, ImportSerialResults *pResults)
{
	HBITMAP hbmWatermark = ::LoadBitmap(AfxGetResourceHandle(), MAKEINTRESOURCE(IDB_IMPORT_SERIAL_WATERMARK));
    HPALETTE hpalWatermark = NULL;
    HBITMAP hbmHeader = ::LoadBitmap(AfxGetResourceHandle(), MAKEINTRESOURCE(IDB_IMPORT_SERIAL_HEADER));

	CImportSerialWizard wizard(pSettings, pResults, IDS_IMPORT_SERIAL_WIZARD_CAPTION, NULL, 0, hbmWatermark, hpalWatermark, hbmHeader);
	INT_PTR result = wizard.DoModal();

	return result;
}

BEGIN_MESSAGE_MAP(CImportSerialWizard, CPropertySheet)
END_MESSAGE_MAP()

// CImportSerialWizard message handlers
