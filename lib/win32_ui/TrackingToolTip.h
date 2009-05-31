class CTrackingToolTip
{
	CToolTipCtrl *m_pCtrl;
	CString m_strText;

public:
	CTrackingToolTip()
		: m_pCtrl(NULL)
	{
	}

	void Show(CWnd *pWnd, CPoint ptScreen, LPCTSTR lpszText);
	void Hide();

	void PreTranslateMessage(MSG *pMsg)
	{
		if (m_pCtrl)
			m_pCtrl->RelayEvent(pMsg);
	}

private:
	BOOL Create(CWnd *pWnd);
	void SetPos(CWnd *pWnd, CPoint ptScreen);

private:
	void FillToolInfo(CWnd *pWnd, TOOLINFO *ti);
};
