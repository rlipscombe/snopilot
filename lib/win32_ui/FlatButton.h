#pragma once


// CFlatButton

class CFlatButton : public CButton
{
	DECLARE_DYNAMIC(CFlatButton)

public:
	CFlatButton();
	virtual ~CFlatButton();

protected:
	virtual void PreSubclassWindow();

protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual void DrawItem(LPDRAWITEMSTRUCT /*lpDrawItemStruct*/);
};


