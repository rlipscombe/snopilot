#include "StdAfx.h"
#include "draw_selection.h"

/** Highlight the selection rectangle.
 */
void DrawSelection(CDC *pDC, const CRect &rect, COLORREF fillColor, COLORREF borderColor)
{
	// We need a DC, and some bits to back it up.  If we had a DIB section, we could do this in place
	// on the memory DC.
	CDC blendDC;
	VERIFY(blendDC.CreateCompatibleDC(pDC));
	CBitmap blendBitmap;
	VERIFY(blendBitmap.CreateCompatibleBitmap(pDC, rect.Width(), rect.Height()));
	CBitmap *oldBlendBitmap = blendDC.SelectObject(&blendBitmap);
	blendDC.FillSolidRect(0, 0, rect.Width(), rect.Height(), fillColor);

	// At this point, 'blend' contains a solid swatch of COLOR_MENUHILIGHT.

	// We can use the AlphaBlend function to draw it nicely.
	BLENDFUNCTION blend;
	blend.BlendOp = AC_SRC_OVER;
	blend.BlendFlags = 0;
	blend.SourceConstantAlpha = 70;
	blend.AlphaFormat = 0;
	VERIFY(pDC->AlphaBlend(rect.left, rect.top, rect.Width(), rect.Height(),
								&blendDC, 0, 0, rect.Width(), rect.Height(), blend));

	blendDC.SelectObject(oldBlendBitmap);

	// Then we'll highlight the edges.
	CRect border(rect);
	pDC->FillSolidRect(border.left, border.top, border.Width(), 1, borderColor);
	pDC->FillSolidRect(border.left, border.bottom-1, border.Width(), 1, borderColor);
	pDC->FillSolidRect(border.left, border.top, 1, border.Height(), borderColor);
	pDC->FillSolidRect(border.right-1, border.top, 1, border.Height(), borderColor);
}

