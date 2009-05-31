#pragma once

class PostFileSettings;
class PostFileResults;

// CPostFileWizard
class CPostFileWizard : public CPropertySheet
{
	HICON m_hIcon;

	DECLARE_DYNAMIC(CPostFileWizard)

public:
	static INT_PTR DoWizard(LPCTSTR lpszLocalFile, CWnd **ppWnd);

private:
	CPostFileWizard(PostFileSettings *pSettings, PostFileResults *pResults,
					UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage, HBITMAP hbmWatermark, HPALETTE hpalWatermark, HBITMAP hbmHeader);
	virtual ~CPostFileWizard();

protected:
	DECLARE_MESSAGE_MAP()

	virtual void PreSubclassWindow();
	virtual BOOL OnInitDialog();
};
