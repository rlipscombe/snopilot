#pragma once


// CTextHyperlink

class CTextHyperlink : public CStatic
{
	CFont m_font;
	HCURSOR m_hCursor;

	DECLARE_DYNAMIC(CTextHyperlink)

public:
	CTextHyperlink();
	virtual ~CTextHyperlink();

protected:
	afx_msg HBRUSH CtlColor(CDC *pDC, UINT nCtlColor);
	afx_msg void OnClicked();
	afx_msg BOOL OnSetCursor(CWnd *pWnd, UINT nHitTest, UINT message);

	DECLARE_MESSAGE_MAP()
};
