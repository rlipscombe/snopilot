#include "StdAfx.h"
#include "SnoTrackSplitterWnd.h"

#define CX_BORDER   1
#define CY_BORDER   1

CSnoTrackSplitterWnd::CSnoTrackSplitterWnd()
{
	m_cxBorder = m_cyBorder = 0;

	m_fixedSize.cx = 250;
	m_fixedSize.cy = 150;
}

BOOL CSnoTrackSplitterWnd::ReplaceView(int row, int col, CRuntimeClass *pViewClass, SIZE size)
{
	CCreateContext context;
	BOOL bSetActive;

	// It's not quite as simple as this.  Essentially, this tries to avoid replacing
	// the view if it's already the same type as the existing one.
	// Where it goes wrong is that IsKindOf also allows for derived classes.  We need
	// to check for equivalence.
	if (GetPane(row, col)->IsKindOf(pViewClass))
		return FALSE;

	// Get pointer to CDocument object so that it can be used in the creation 
	// process of the new view
	CDocument *pDoc = ((CView *)GetPane(row, col))->GetDocument();
	CView *pActiveView = GetParentFrame()->GetActiveView();
	if (pActiveView == NULL || pActiveView == GetPane(row, col))
		bSetActive = TRUE;
	else
		bSetActive = FALSE;

	// set flag so that document will not be deleted when view is destroyed
	pDoc->m_bAutoDelete = FALSE;    
	// Delete existing view 
	((CView *) GetPane(row,col))->DestroyWindow();
	// set flag back to default 
	pDoc->m_bAutoDelete = TRUE;

	// Create new view                      

	context.m_pNewViewClass = pViewClass;
	context.m_pCurrentDoc = pDoc;
	context.m_pNewDocTemplate = NULL;
	context.m_pLastView = NULL;
	context.m_pCurrentFrame = NULL;

	CreateView(row, col, pViewClass, size, &context);

	CView *pNewView = (CView *)GetPane(row, col);

	if (bSetActive == TRUE)
		GetParentFrame()->SetActiveView(pNewView);

	RecalcLayout(); 
	GetPane(row, col)->SendMessage(WM_INITIALUPDATE);

	return TRUE;
}

void CSnoTrackSplitterWnd::OnDrawSplitter(CDC* pDC, ESplitType nType,
	const CRect& rectArg)
{
	// if pDC == NULL, then just invalidate
	if (pDC == NULL)
	{
		RedrawWindow(rectArg, NULL, RDW_INVALIDATE|RDW_NOCHILDREN);
		return;
	}
	ASSERT_VALID(pDC);

	// otherwise, actually draw
	CRect rect = rectArg;
	switch (nType)
	{
	case splitBorder:
//		pDC->Draw3dRect(rect, afxData.clrBtnShadow, afxData.clrBtnHilite);
//		rect.InflateRect(-CX_BORDER, -CY_BORDER);
//		pDC->Draw3dRect(rect, afxData.clrWindowFrame, afxData.clrBtnFace);
		return;

	case splitIntersection:
		break;

	case splitBox:
//		pDC->Draw3dRect(rect, afxData.clrBtnFace, afxData.clrWindowFrame);
		pDC->Draw3dRect(rect, RGB(255,0,0), RGB(0,255,255));
		rect.InflateRect(-CX_BORDER, -CY_BORDER);
//		pDC->Draw3dRect(rect, afxData.clrBtnHilite, afxData.clrBtnShadow);
		pDC->Draw3dRect(rect, RGB(0,0,255), RGB(255,255,0));
		rect.InflateRect(-CX_BORDER, -CY_BORDER);
		break;

	case splitBar:
		break;

	default:
		ASSERT(FALSE);  // unknown splitter type
	}

	// fill the middle
	COLORREF clr = GetSysColor(COLOR_HIGHLIGHT);
	pDC->FillSolidRect(rect, clr);
}

BEGIN_MESSAGE_MAP(CSnoTrackSplitterWnd, CSplitterWnd)
	ON_WM_SIZE()
END_MESSAGE_MAP()

void CSnoTrackSplitterWnd::StopTracking(BOOL bAccept)
{
	CSplitterWnd::StopTracking(bAccept);

	if (bAccept)
	{
		if (m_nRows > 1 && m_pRowInfo)
			m_fixedSize.cy = m_pRowInfo[1].nCurSize;

		if (m_nCols > 1 && m_pColInfo)
			m_fixedSize.cx = m_pColInfo[1].nCurSize;
	}
}

void CSnoTrackSplitterWnd::OnSize(UINT nType, int cx, int cy)
{
	CRect insideRect;
	GetInsideRect(insideRect);

	// Before allowing the base class to handle it, we'll tweak the numbers.
	if (m_nRows > 1 && m_pRowInfo)
	{
		// We're a horizontal splitter.
		m_pRowInfo[0].nIdealSize = insideRect.Height() - m_fixedSize.cy - m_cySplitterGap;
	}

	if (m_nCols > 1 && m_pColInfo)
	{
		// We're a vertical splitter.
		m_pColInfo[0].nIdealSize = insideRect.Width() - m_fixedSize.cx - m_cxSplitterGap;
	}

	CSplitterWnd::OnSize(nType, cx, cy);
}
