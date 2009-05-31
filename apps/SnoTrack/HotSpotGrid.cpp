#include "StdAfx.h"
#include "HotspotGrid.h"
#include "GridBounds.h"	// For RoundUp/RoundDown

HotspotGrid::HotspotGrid()
	: m_hotspotCount(0), m_cellSpacing(100)
{
}

bool HotspotGrid::Create(const CRect &logicalRange, int cellSpacing)
{
	m_cellSpacing = cellSpacing;
	m_range = ExpandLogicalRange(logicalRange, m_cellSpacing);

	int cellCount = GetWidthInCells() * GetHeightInCells();
	m_bits.resize(cellCount, false);

	return true;
}

bool HotspotGrid::GetCellFromLogicalPoint(CPoint logicalPoint, int *xCell, int *yCell) const
{
	ASSERT(xCell);
	ASSERT(yCell);

	if (m_range.PtInRect(logicalPoint))
	{
		int dx = logicalPoint.x - m_range.left;
		int dy = logicalPoint.y - m_range.top;

		*xCell = dx / m_cellSpacing;
		*yCell = dy / m_cellSpacing;

		return true;
	}
	else
		return false;
}

void HotspotGrid::AddHotspot(int xCell, int yCell, int hotspotId)
{
	if (xCell >= 0 && xCell < GetWidthInCells() && yCell >= 0 && yCell < GetHeightInCells())
	{
		int bitNumber = GetCellIndex(xCell, yCell);
		if (m_bits[bitNumber])
			return;	// Already set.

		m_bits[bitNumber] = true;
		m_ids[CellRef(xCell, yCell)] = hotspotId;

		++m_hotspotCount;
	}
}

void HotspotGrid::AddHotspot(CPoint logicalPoint, int hotspotId)
{
	int xCell, yCell;

	if (GetCellFromLogicalPoint(logicalPoint, &xCell, &yCell))
		AddHotspot(xCell, yCell, hotspotId);
}

void HotspotGrid::AddHotspot(CPoint logicalPoint, int hotspotId, int size)
{
	int xCell, yCell;

	if (GetCellFromLogicalPoint(logicalPoint, &xCell, &yCell))
	{
		for (int x = xCell - size; x <= xCell + size; ++x)
		{
			for (int y = yCell - size; y <= yCell + size; ++y)
			{
				AddHotspot(x, y, hotspotId);
			}
		}
	}
}

void HotspotGrid::DeleteHotspot(int xCell, int yCell)
{
	if (xCell >= 0 && xCell < GetWidthInCells() && yCell >= 0 && yCell < GetHeightInCells())
	{
		int bitNumber = GetCellIndex(xCell, yCell);
		if (!m_bits[bitNumber])
			return;	// Not set.

		m_bits[bitNumber] = false;

		m_ids.erase(CellRef(xCell, yCell));

		--m_hotspotCount;
	}
}

void HotspotGrid::DeleteHotspot(CPoint logicalPoint)
{
	int xCell, yCell;

	if (GetCellFromLogicalPoint(logicalPoint, &xCell, &yCell))
		DeleteHotspot(xCell, yCell);
}

void HotspotGrid::DeleteHotspot(CPoint logicalPoint, int size)
{
	int xCell, yCell;

	if (GetCellFromLogicalPoint(logicalPoint, &xCell, &yCell))
	{
		for (int x = xCell - size; x <= xCell + size; ++x)
		{
			for (int y = yCell - size; y <= yCell + size; ++y)
			{
				DeleteHotspot(x, y);
			}
		}
	}
}

void HotspotGrid::ToggleHotspot(CPoint logicalPoint, int hotspotId, int size)
{
	int id;
	if (GetHotspotFromLogicalPoint(logicalPoint, &id))
	{
		DeleteHotspot(logicalPoint, size-1);
		TRACE("Deleted hotspotId %d\n", id);
	}
	else
	{
		AddHotspot(logicalPoint, hotspotId, size-1);
		TRACE("Added hotspotId %d\n", hotspotId);
	}
}

bool HotspotGrid::GetHotspotFromCell(int xCell, int yCell, int *hotspotId) const
{
	if (xCell >= 0 && xCell < GetWidthInCells() && yCell >= 0 && yCell < GetHeightInCells())
	{
		int bitNumber = GetCellIndex(xCell, yCell);
		if (!m_bits[bitNumber])
			return false;	// Not set.

		ids_t::const_iterator i = m_ids.find(CellRef(xCell, yCell));
		ASSERT(i != m_ids.end());	// bitset and map get out of step?

		*hotspotId = i->second;
		return true;
	}
	else
	{
		ASSERT(FALSE);
		return false;
	}
}

bool HotspotGrid::GetHotspotFromLogicalPoint(CPoint logicalPoint, int *hotspotId) const
{
	int xCell, yCell;

	if (GetCellFromLogicalPoint(logicalPoint, &xCell, &yCell))
		return GetHotspotFromCell(xCell, yCell, hotspotId);
	else
		return false;
}

/* static */
CRect HotspotGrid::ExpandLogicalRange(const CRect &logicalRange, int cellSpacing)
{
	return CRect(RoundDown(logicalRange.left, cellSpacing), RoundDown(logicalRange.top, cellSpacing),
					RoundUp(logicalRange.right, cellSpacing), RoundUp(logicalRange.bottom, cellSpacing));
}

CRect HotspotGrid::GetCellRect(int xCell, int yCell) const
{
	CRect rect;
	rect.left = m_range.left + (xCell * GetCellSpacing());
	rect.top = m_range.top + (yCell * GetCellSpacing());
	rect.right = rect.left + GetCellSpacing();
	rect.bottom = rect.top + GetCellSpacing();

	return rect;
}

void HotspotGrid::EnumerateHotspots(HotspotGridCallback *pCallback) const
{
	DWORD startTime = GetTickCount();

	for (ids_t::const_iterator i = m_ids.begin(); i != m_ids.end(); ++i)
	{
		CellRef cellRef = i->first;
		int hotspotId = i->second;

		CRect rect = GetCellRect(cellRef.x, cellRef.y);
		pCallback->OnHotspot(cellRef.x, cellRef.y, hotspotId, rect);
	}

	DWORD endTime = GetTickCount();

	TRACE("EnumerateHotspots took %d ms\n", endTime - startTime);
}

int HotspotGrid::SuggestNextHotspotID() const
{
	int nextHotspotID = -1;
	for (ids_t::const_iterator i = m_ids.begin(); i != m_ids.end(); ++i)
	{
		int hotspotId = i->second;

		nextHotspotID = max(nextHotspotID, hotspotId);
	}

	return nextHotspotID + 1;
}
