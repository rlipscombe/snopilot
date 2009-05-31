// IntroView.cpp : implementation file
//

#include "stdafx.h"
#include "IntroView.h"
#include "resource.h"
#include "win32_ui/bold_font.h"
#include "win32_ui/center_rectangle.h"
#include "ChildFrm.h"
#include "SnoTrackDoc.h"
#include "CartesianView.h"

// CIntroView
IMPLEMENT_DYNCREATE(CIntroView, CView)

CIntroView::CIntroView()
{
}

CIntroView::~CIntroView()
{
}

BEGIN_MESSAGE_MAP(CIntroView, CView)
END_MESSAGE_MAP()


// CIntroView drawing

void CIntroView::OnDraw(CDC* pDC)
{
	CRect clientRect;
	GetClientRect(&clientRect);

	LOGFONT lf;
	GetMessageBoxFont(&lf);

	CFont font;
	font.CreateFontIndirect(&lf);
	CFont *oldFont = pDC->SelectObject(&font);

	CString intro;
	VERIFY(intro.LoadString(IDS_EMPTY_DATASET));
	CSize textSize = pDC->GetTextExtent(intro);

	CRect textRect = CenterRectangle(clientRect, textSize);

	pDC->SetTextColor(GetSysColor(COLOR_WINDOWTEXT));
	pDC->SetBkColor(GetSysColor(COLOR_WINDOW));
	pDC->ExtTextOut(textRect.left, textRect.top, ETO_CLIPPED | ETO_OPAQUE, clientRect, intro, NULL);

	pDC->SelectObject(oldFont);
}


// CIntroView diagnostics

#ifdef _DEBUG
void CIntroView::AssertValid() const
{
	CView::AssertValid();
}

void CIntroView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG


// CIntroView message handlers

void CIntroView::OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/)
{
	CSnoTrackDoc* pDoc = (CSnoTrackDoc*)GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// It's no longer empty.
	if (!pDoc->IsEmptyDataSet())
	{
		CChildFrame *pFrame = static_cast<CChildFrame *>(GetParentFrame());
		pFrame->DeferReplaceView(RUNTIME_CLASS(CCartesianView));
	}
}
