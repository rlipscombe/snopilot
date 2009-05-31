/* column.h
 *
 * Copyright (C) 2003 Roger Lipscombe, http://www.differentpla.net/~roger/
 */

#ifndef COLUMN_H
#define COLUMN_H 1

#include "tstring.h"

class CListCtrlItem;

/** Abstract base class that defines the behaviour of a column in a listview control.
 *
 * This adds the ability to define custom formatting for the attributes of each list
 * item, and the ability to sort by other than strict lexicographic order.
 */
class Column
{
	DISALLOW_COPYING(Column);

protected:
    Column() { /* nothing */ }

public:
    virtual tstring GetHeading() const = 0;
    virtual int GetFormat() const = 0;
    virtual int GetDefaultWidth() const = 0;
    virtual int GetSubItem() const = 0;

    /** Return user-visible formatted description of the item. */
    virtual tstring GetText(const CListCtrlItem *pItem) const = 0;

    /** Compare two items.  Return <0, 0 or >0. */
    virtual int Compare(const CListCtrlItem *lhs, const CListCtrlItem *rhs) const = 0;
};

#endif /* COLUMN_H */
