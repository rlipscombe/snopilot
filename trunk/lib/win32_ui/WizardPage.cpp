#include "StdAfx.h"
#include "WizardPage.h"
#include "bold_font.h"

IMPLEMENT_DYNAMIC(CWizardPage, CPropertyPage);

bool CWizardPage::CreateBoldFont()
{
	LOGFONT lf;
	GetMessageBoxFont(&lf);

	CDC *pDC = GetDC();
	lf.lfHeight = -MulDiv(12, GetDeviceCaps(pDC->m_hDC, LOGPIXELSY), 72);
	ReleaseDC(pDC);

	lf.lfWeight = FW_BOLD;

	if (m_boldFont.CreateFontIndirect(&lf))
		return true;
	return false;
}

void CWizardPage::InflictBoldFont(UINT nIDC)
{
	if (!m_boldFont.GetSafeHandle())
		VERIFY(CreateBoldFont());

	CWnd *pWnd = GetDlgItem(nIDC);
	if (pWnd)
		pWnd->SetFont(&m_boldFont);
}
