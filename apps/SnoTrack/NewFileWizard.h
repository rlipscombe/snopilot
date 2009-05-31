#pragma once

class NewFileSettings;

// CNewFileWizard

class CNewFileWizard : public CPropertySheet
{
	DECLARE_DYNAMIC(CNewFileWizard)

private:
	CNewFileWizard(NewFileSettings *pSettings, UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage, HBITMAP hbmWatermark, HPALETTE hpalWatermark, HBITMAP hbmHeader);
	virtual ~CNewFileWizard();

public:
	static INT_PTR DoWizard(NewFileSettings *pSettings);

protected:
	DECLARE_MESSAGE_MAP()
};


