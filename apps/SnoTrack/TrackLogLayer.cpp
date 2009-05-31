#include "StdAfx.h"
#include "TrackLogLayer.h"
#include "DrawSite.h"
#include <algorithm>

int CalculateLineWidth(PositionFix::Style lineStyle)
{
	switch (lineStyle)
	{
	case PositionFix::STYLE_FADED:
	case PositionFix::STYLE_NORMAL:
	default:
		return 1;

	case PositionFix::STYLE_HIGHLIGHT:
		return 2;
	}
}

// For the colour, there's 4 different cases, based on secondsSinceLast.
int CalculateColourColumn(ULONGLONG secondsSinceLast)
{
	if (secondsSinceLast <= 5)
		return 0;
	else if (secondsSinceLast <= 10)
		return 1;
	else if (secondsSinceLast <= 15)
		return 2;
	else
		return 3;
}

int CalculateColourRow(PositionFix::Style lineStyle)
{
	switch (lineStyle)
	{
	case PositionFix::STYLE_NORMAL:
	default:
		return 0;

	case PositionFix::STYLE_FADED:
		return 1;

	case PositionFix::STYLE_HIGHLIGHT:
		return 2;
	}
}

COLORREF CalculateLineColour(PositionFix::Style lineStyle, ULONGLONG secondsSinceLast)
{
	// This calls for a table-based solution:
	COLORREF colourTable[3][4] = {
		//     <= 5                     <= 10            <= 15                 <= 20
		{ RGB(0x00,0x00,0x00), RGB(0x00,0x00,0xFF), RGB(0x00,0xFF,0x00), RGB(0xFF,0x00,0x00), },	// normal
		{ RGB(0x80,0x80,0x80), RGB(0x80,0x80,0xFF), RGB(0x80,0xFF,0x80), RGB(0xFF,0x80,0x80), },	// faded
		{ RGB(0x00,0x00,0x00), RGB(0x00,0x00,0xFF), RGB(0x00,0xFF,0x00), RGB(0xFF,0x00,0x00), },	// highlight
		// Black                      Blue                Green               Red
	};

	int colourCol = CalculateColourColumn(secondsSinceLast);
	int colourRow = CalculateColourRow(lineStyle);

	return colourTable[colourRow][colourCol];
}

void TrackLogLayer::OnRecalc(const CRect &logicalRange)
{
	DWORD startTime = GetTickCount();

	DestroyBsp();

	if (m_positions->size() < 2)
		return;		// Not enough data to bother with.

	CreateBsp(logicalRange);

	FillBsp();

	DWORD endTime = GetTickCount();
	TRACE("TrackLogLayer::OnRecalc took %d ms\n", endTime - startTime);
}

class SelectionCallback : public BspWalkCallback
{
	const SelectionTimeSpanCollection &m_selection;

public:
	SelectionCallback(const SelectionTimeSpanCollection &selection)
		: m_selection(selection)
	{
	}

	class HighlightItem
	{
		const SelectionTimeSpan &m_selection;

	public:
		HighlightItem(const SelectionTimeSpan &selection)
			: m_selection(selection)
		{
		}

		void operator() (LineDef &line)
		{
			PositionFix::Style style = PositionFix::STYLE_FADED;
			
			if (m_selection.m_startTime <= line.m_timestamp && line.m_timestamp < m_selection.m_endTime)
				style = PositionFix::STYLE_HIGHLIGHT;

			line.m_cr = CalculateLineColour(style, line.m_gap);
			line.m_width = CalculateLineWidth(style);
		}
	};

	class HighlightItems
	{
		items_t *m_items;

	public:
		HighlightItems(items_t *items)
			: m_items(items)
		{
		}

		void operator() (const SelectionTimeSpan &selection)
		{
			std::for_each(m_items->begin(), m_items->end(), HighlightItem(selection));
		}
	};

	class UnhighlightItem
	{
	public:
		void operator() (LineDef &line)
		{
			PositionFix::Style style = PositionFix::STYLE_NORMAL;

			line.m_cr = CalculateLineColour(style, line.m_gap);
			line.m_width = CalculateLineWidth(style);
		}
	};

	virtual void OnLeafNode(items_t *items)
	{
		if (m_selection.empty())
			std::for_each(items->begin(), items->end(), UnhighlightItem());
		else
			std::for_each(m_selection.begin(), m_selection.end(), HighlightItems(items));
	}
};

void TrackLogLayer::OnSelect(const SelectionTimeSpanCollection &selection)
{
//	DWORD startTime = GetTickCount();

	// Walk the BSP, looking for linedefs that fall into the selection range.
	SelectionCallback cb(selection);
	WalkBsp(&cb);

//	DWORD endTime = GetTickCount();
//	TRACE("TrackLogLayer::OnSelect took %d ms\n", endTime - startTime);
}

void TrackLogLayer::FillBsp()
{
	DWORD startTime = GetTickCount();

	// Generate a bunch of LineDefs from the PositionFix collection.
	// TODO: This could use a unit test, probably.
	PositionFixCollection::const_iterator b = m_positions->begin();
	PositionFixCollection::const_iterator e = m_positions->end();
	ASSERT(b != e);	// no positions?

	for (PositionFixCollection::const_iterator i = b+1; i != e; ++i)
	{
		PositionFixCollection::const_iterator prev_fix = i-1;
		PositionFixCollection::const_iterator this_fix = i;

		// If the position fixes are more than 20 seconds apart, don't bother drawing a line between them.
		CFileTime this_timestamp = this_fix->GetTimestamp();
		CFileTime prev_timestamp = prev_fix->GetTimestamp();
		CFileTimeSpan elapsed = this_timestamp - prev_timestamp;

		ULONGLONG secondsSinceLast = elapsed.GetTimeSpan() / CFileTime::Second;
		if (secondsSinceLast <= 20)
		{
			InsertLineDef(prev_fix, this_fix, secondsSinceLast);
		}
//		else
//			TRACE("Skipped line: %d seconds since last fix.\n", secondsSinceLast);
	}

	DWORD endTime = GetTickCount();
	TRACE("TrackLogLayer::FillBsp took %d ms\n", endTime - startTime);
}

void TrackLogLayer::InsertLineDef(PositionFixCollection::const_iterator prev_fix, PositionFixCollection::const_iterator this_fix,
								  ULONGLONG secondsSinceLast)
{
	if (prev_fix->IsHidden() && this_fix->IsHidden())
		return;

	int x1 = prev_fix->GetProjectionX();
	int y1 = prev_fix->GetProjectionY();

	int x2 = this_fix->GetProjectionX();
	int y2 = this_fix->GetProjectionY();

	COLORREF cr = CalculateLineColour(prev_fix->GetStyle(), secondsSinceLast);
	int width = CalculateLineWidth(prev_fix->GetStyle());

	// We want a LineDef between the previous point and this point.
	InsertLineDef(x1, y1, x2, y2, cr, width, prev_fix->GetTimestamp(), secondsSinceLast);
}
