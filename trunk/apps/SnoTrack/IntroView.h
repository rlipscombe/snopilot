#pragma once


// CIntroView view

class CIntroView : public CView
{
	DECLARE_DYNCREATE(CIntroView)

protected:
	CIntroView();           // protected constructor used by dynamic creation
	virtual ~CIntroView();

public:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	DECLARE_MESSAGE_MAP()
	virtual void OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/);
};


