#pragma once

class CWizardPage : public CPropertyPage
{
	DECLARE_DYNAMIC(CWizardPage)

public:
	CWizardPage(UINT nIDTemplate, UINT nIDCaption)
		: CPropertyPage(nIDTemplate, nIDCaption)
	{
	}

	CWizardPage(UINT nIDTemplate, UINT nIDCaption, UINT nIDHeaderTitle, UINT nIDHeaderSubTitle)
		: CPropertyPage(nIDTemplate, nIDCaption, nIDHeaderTitle, nIDHeaderSubTitle)
	{
	}

	inline void SetWizardButtons(DWORD dwFlags);
	inline void EnableCancelButton(BOOL bEnable);

protected:
	void InflictBoldFont(UINT nIDC);

private:
	/// Used for the welcome/complete pages of a wizard.
	CFont m_boldFont;

	bool CreateBoldFont();
};

void CWizardPage::SetWizardButtons(DWORD dwFlags)
{
	static_cast<CPropertySheet *>(GetParent())->SetWizardButtons(dwFlags);
}

void CWizardPage::EnableCancelButton(BOOL bEnable)
{
	CWnd *pCancel = GetParent()->GetDlgItem(IDCANCEL);
	if (pCancel)
		pCancel->EnableWindow(bEnable);
}
