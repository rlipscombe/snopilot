#pragma once


// COwnerDrawListBox

class COwnerDrawListBox : public CListBox
{
	DECLARE_DYNAMIC(COwnerDrawListBox)

public:
	COwnerDrawListBox();
	virtual ~COwnerDrawListBox();

protected:
	virtual void PreSubclassWindow();
	virtual void DrawItem(LPDRAWITEMSTRUCT drawItem);
	virtual void DoDrawItem(CDC *pDC, const void *itemData, const CRect &itemRect, DWORD itemState);

protected:
	DECLARE_MESSAGE_MAP()
};
