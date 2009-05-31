#pragma once



// CImportSerialWizard
class ImportSerialSettings;
class ImportSerialResults;

class CImportSerialWizard : public CPropertySheet
{
	DECLARE_DYNAMIC(CImportSerialWizard)

private:
	// Use DoWizard.
	CImportSerialWizard(ImportSerialSettings *pSettings, ImportSerialResults *pResults, UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage, HBITMAP hbmWatermark, HPALETTE hpalWatermark, HBITMAP hbmHeader);
	virtual ~CImportSerialWizard();

public:
	static INT_PTR DoWizard(ImportSerialSettings *pSettings, ImportSerialResults *pResults);

protected:
	DECLARE_MESSAGE_MAP()
};


