#pragma once

class CSnoTrackSplitterWnd : public CSplitterWnd
{
	CSize m_fixedSize;

public:
	CSnoTrackSplitterWnd();

	BOOL ReplaceView(int row, int col, CRuntimeClass *pViewClass, SIZE size);
	
protected:
	virtual void OnDrawSplitter(CDC* pDC, ESplitType nType, const CRect& rect);
	virtual void StopTracking(BOOL bAccept);

	afx_msg void OnSize(UINT nType, int cx, int cy);

	DECLARE_MESSAGE_MAP()
};
