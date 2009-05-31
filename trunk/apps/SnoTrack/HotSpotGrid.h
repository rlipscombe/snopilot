#pragma once
#include <vector>
#include <map>

// Figuring out which runs/lifts/rests someone used is done with a bunch of "hotspots".
// These are defined in the resort file.  For example, there might be a hotspot at
// the bottom of a lift.  We can then tell when the user got on the lift because their
// tracklog goes near the hotspot.
//
// We can differentiate between going near the lift and travelling on the lift by
// placing multiple hotspots on the lift (e.g. another one half way up, and one at
// the top).
//
// Then, for example, if the user hits spots 1, 3 and 5, we know they went up the lift.
// If they hit 5, 4 and 1, we know they came down on a particular run from the lift.
// If they hit 5, 3, 2 and 1, we know they came down a different run that goes under the lift.
//
// For quick lookup, the implementation uses a grid of hotspot cells.  Each of these can have
// zero or one hotspots defined for it.

class HotspotGridCallback;

class HotspotGrid
{
	CRect m_range;
	int m_cellSpacing;

	typedef std::vector< bool > bits_t;
	bits_t m_bits;

	struct CellRef {

		CellRef(int xx, int yy)
			: x(xx), y(yy)
		{
		}

		int x; int y;

		/** We need to define a total ordering on CellRefs.  The easiest way
		 * to do this is to first check y, and then check x.
		 */
		bool operator< (const CellRef &other) const
		{
			if (y < other.y)
				return true;

			// So y >= other.y.  If it's greater, then the other one is less.
			if (y > other.y)
				return false;

			// So y == other.y.  Compare on x:
			return (x < other.x);
		}
	};

	typedef std::map< CellRef, int > ids_t;
	ids_t m_ids;

	int m_hotspotCount;

public:
	static const int INVALID_HOTSPOT_ID = -1;

	HotspotGrid();

	bool Create(const CRect &logicalRange, int cellSpacing);

	int GetWidthInCells() const;
	int GetHeightInCells() const;

	int GetHotspotCount() const { return m_hotspotCount; }
	int SuggestNextHotspotID() const;

	CRect GetRange() const { return m_range; }
	int GetCellSpacing() const { return m_cellSpacing; }

	bool GetHotspotFromLogicalPoint(CPoint logicalPoint, int *hotspotId) const;
	bool GetHotspotFromCell(int xCell, int yCell, int *hotspotId) const;

	void ToggleHotspot(CPoint logicalPoint, int hotspotId, int size = 1);

	void AddHotspot(CPoint logicalPoint, int hotspotId);
	void AddHotspot(int xCell, int yCell, int hotspotId);
	void AddHotspot(CPoint logicalPoint, int hotspotId, int size);

	void DeleteHotspot(CPoint logicalPoint);
	void DeleteHotspot(int xCell, int yCell);
	void DeleteHotspot(CPoint logicalPoint, int size);

	void EnumerateHotspots(HotspotGridCallback *pCallback) const;

private:
	bool GetCellFromLogicalPoint(CPoint logicalPoint, int *xCell, int *yCell) const;

	int GetCellIndex(int xCell, int yCell) const;
	CRect GetCellRect(int xCell, int yCell) const;

	static CRect ExpandLogicalRange(const CRect &logicalRange, int cellSpacing);
};

inline int HotspotGrid::GetWidthInCells() const
{
	return m_range.Width() / m_cellSpacing;
}

inline int HotspotGrid::GetHeightInCells() const
{
	return m_range.Height() / m_cellSpacing;
}

inline int HotspotGrid::GetCellIndex(int xCell, int yCell) const
{
	return (yCell * GetWidthInCells()) + xCell;
}

class HotspotGridCallback
{
public:
	virtual void OnHotspot(int xCell, int yCell, int hotspotId, CRect logicalRect) = 0;
};

void test_HotspotGrid();
