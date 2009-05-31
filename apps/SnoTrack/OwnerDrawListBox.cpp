// OwnerDrawListBox.cpp : implementation file
//

#include "stdafx.h"
#include "SnoTrack.h"
#include "OwnerDrawListBox.h"

// COwnerDrawListBox
IMPLEMENT_DYNAMIC(COwnerDrawListBox, CListBox)

COwnerDrawListBox::COwnerDrawListBox()
{
}

COwnerDrawListBox::~COwnerDrawListBox()
{
}


BEGIN_MESSAGE_MAP(COwnerDrawListBox, CListBox)
END_MESSAGE_MAP()

/** Some cunning code to work around the fact that LBS_OWNERDRAWFIXED
 * listboxes don't see the reflected WM_MEASUREITEM at creation time
 * because they've not been subclassed yet.
 */
void COwnerDrawListBox::PreSubclassWindow()
{
	CListBox::PreSubclassWindow();

	if (GetStyle() & LBS_OWNERDRAWFIXED)
	{
		MEASUREITEMSTRUCT measureItem;

		// MeasureItem will probably ignore these, but we'll fill them in anyway:
		measureItem.CtlID = GetDlgCtrlID();
		measureItem.CtlType = ODT_LISTBOX;
		measureItem.itemID = -1;
		measureItem.itemData = 0;

		// MeasureItem is supposed to fill these in, but we'll put some defaults in there:
		measureItem.itemWidth = 0;
		measureItem.itemHeight = 0;

		MeasureItem(&measureItem);

		// If we're LBS_OWNERDRAWFIXED, then the item ID should be zero.
		SetItemHeight(0, measureItem.itemHeight);
	}
}

/** Off-screen drawing to avoid flicker.
 */
void COwnerDrawListBox::DrawItem(LPDRAWITEMSTRUCT drawItem)
{
	const void *itemData = reinterpret_cast<const void *>(drawItem->itemData);
	if (!itemData)
		return;		// The documentation doesn't talk about it; but it does happen.

	CRect itemRect(drawItem->rcItem);

	CDC dc;
	dc.Attach(drawItem->hDC);

	CDC memoryDC;
	VERIFY(memoryDC.CreateCompatibleDC(&dc));

	CBitmap memoryBitmap;
	VERIFY(memoryBitmap.CreateCompatibleBitmap(&dc, itemRect.Width(), itemRect.Height()));

	CBitmap *oldMemoryBitmap = memoryDC.SelectObject(&memoryBitmap);

	CRect memoryItemRect(CPoint(0,0), itemRect.Size());
	DoDrawItem(&memoryDC, itemData, memoryItemRect, drawItem->itemState);

	dc.BitBlt(itemRect.left, itemRect.top, itemRect.Width(), itemRect.Height(), &memoryDC, 0, 0, SRCCOPY);

	memoryDC.SelectObject(oldMemoryBitmap);

	dc.Detach();
}

void COwnerDrawListBox::DoDrawItem(CDC *pDC, const void *itemData, const CRect &itemRect, DWORD itemState)
{
	// Do nothing.  The derived class should override this function.
	// The reason that it's not pure is because the derived class might choose to override DrawItem,
	// in which case they'd have no need to replace this function.
	ASSERT(false);
}
