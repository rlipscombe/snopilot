/* ListCtrlItem.h
 *
 * Copyright (C) 2003 Roger Lipscombe, http://www.differentpla.net/~roger/
 */

#ifndef LISTCTRLITEM_H
#define LISTCTRLITEM_H 1

class CListCtrlItem
{
public:
	virtual ~CListCtrlItem() { }

    int GetState() const { return 0; }
    int GetStateMask() const { return 0; }

    virtual int GetImage() const { return 0; }
};

#endif /* LISTCTRLITEM_H */
