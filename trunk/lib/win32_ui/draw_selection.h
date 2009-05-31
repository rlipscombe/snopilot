#pragma once

/** Highlight the selection rectangle.
 */
void DrawSelection(CDC *pDC, const CRect &rect, COLORREF fillColor, COLORREF borderColor);

inline void DrawSelection(CDC *pDC, const CRect &rect)
{
	DrawSelection(pDC, rect, GetSysColor(COLOR_MENUHILIGHT), GetSysColor(COLOR_HIGHLIGHT));
}
