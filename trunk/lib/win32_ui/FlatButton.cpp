// FlatButton.cpp : implementation file
//

#include "stdafx.h"
#include "FlatButton.h"
#include "bold_font.h"
#include "center_rectangle.h"

// CFlatButton

IMPLEMENT_DYNAMIC(CFlatButton, CButton)
CFlatButton::CFlatButton()
{
}

CFlatButton::~CFlatButton()
{
}


BEGIN_MESSAGE_MAP(CFlatButton, CButton)
END_MESSAGE_MAP()

// CFlatButton message handlers
void CFlatButton::PreSubclassWindow()
{
	CButton::PreSubclassWindow();

	SendMessage(BM_SETSTYLE, BS_OWNERDRAW, TRUE);
}

void CFlatButton::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	ASSERT(lpDrawItemStruct->CtlType == ODT_BUTTON);

	// lpDrawItemStruct->itemAction tells us what's changed.
	// For now, we'll ignore it and draw the whole thing anyway.

	// That's contained in lpDrawItemStruct->itemState.
	
	// OK, for now, we'll just draw the whole thing.
	CDC dc;
	dc.Attach(lpDrawItemStruct->hDC);

	LOGFONT lf;
	GetMessageBoxFont(&lf);

	lf.lfWeight = FW_BOLD;

	CFont font;
	font.CreateFontIndirect(&lf);
	
	CFont *oldFont = dc.SelectObject(&font);

	CRect rcItem(&lpDrawItemStruct->rcItem);

	CString text;
	GetWindowText(text);

	CSize textSize = dc.GetTextExtent(text);
	
	// The text needs to be centered in the window.
	CRect textRect(CPoint(0, 0), textSize);
	textRect = CenterRectangle(rcItem, textRect);

	dc.SetBkColor(RGB(238,238,220));
	dc.SetTextColor(RGB(36,98,170));
	dc.ExtTextOut(textRect.left, textRect.top, ETO_CLIPPED | ETO_OPAQUE, rcItem, text, NULL);

	// Draw a border around the thing.
	dc.FillSolidRect(rcItem.left, rcItem.top, rcItem.Width(), 1, RGB(170,170,170));
	dc.FillSolidRect(rcItem.left, rcItem.top, 1, rcItem.Height(), RGB(170,170,170));
	dc.FillSolidRect(rcItem.right-1, rcItem.top, 1, rcItem.Height(), RGB(170,170,170));
	dc.FillSolidRect(rcItem.left, rcItem.bottom-1, rcItem.Width(), 1, RGB(170,170,170));

	if (lpDrawItemStruct->itemState & ODS_FOCUS)
	{
		CRect focusRect(rcItem);
		focusRect.DeflateRect(4, 4, 4, 4);
		dc.DrawFocusRect(focusRect);
	}

	dc.SelectObject(oldFont);
	dc.Detach();
}
