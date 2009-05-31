#include "StdAfx.h"
#include "HotspotGrid.h"
#include "test/test_assert.h"

class TestHotspotGridCallback : public HotspotGridCallback
{
	int m_count;
	CRect m_rect;

public:
	TestHotspotGridCallback()
		: m_count(0), m_rect(0,0,0,0)
	{
	}

	int GetCount() const { return m_count; }
	CRect GetRect() const { return m_rect; }

	virtual void OnHotspot(int xCell, int yCell, int hotspotId, CRect logicalRect)
	{
		++m_count;
		m_rect = logicalRect;
	}
};

void test_HotspotGrid()
{
	// So, the first thing that we need to do is test that we can set up a grid for a particular
	// resort:

	HotspotGrid grid;

	// Units in the logical range are /10cm.  The default hotspot grid size is 10m, so 100 units.
	// This range defines a space 10000 units across (1km), so it'll have 100 cells in each direction.
	CRect logicalRange(-5000, -5000, 5000, 5000);
	assertTrue(grid.Create(logicalRange, 100));

	int expectedSize = logicalRange.Width() / grid.GetCellSpacing();

	assertIntegersEqual(expectedSize, grid.GetWidthInCells());
	assertIntegersEqual(expectedSize, grid.GetHeightInCells());

	assertTrue(logicalRange == grid.GetRange());

	// If the user clicks on a cell, we need to know whether there's a hotspot there yet:
	// None of the cells should have a hotspot defined in them yet.
	for (int x = 0; x < expectedSize; ++x)
	{
		for (int y = 0; y < expectedSize; ++y)
		{
			int id;
			assertFalse(grid.GetHotspotFromCell(x, y, &id));
		}
	}

	// If the user clicks on a cell, we need to be able to define a hotspot for that cell:
	int expectedID = (rand() << 24) | (rand() << 16) | (rand() << 8) | rand();
	grid.AddHotspot(3, 5, expectedID);

	assertIntegersEqual(1, grid.GetHotspotCount());

	int actualID;
	assertTrue(grid.GetHotspotFromCell(3, 5, &actualID));
	assertIntegersEqual(expectedID, actualID);

	TestHotspotGridCallback cb;
	grid.EnumerateHotspots(&cb);
	assertIntegersEqual(1, cb.GetCount());
//	assertTrue(CRect(-2000, 0, -1000, 1000) == cb.GetRect());

	// Try deleting a hotspot
	grid.DeleteHotspot(3, 5);
	assertIntegersEqual(0, grid.GetHotspotCount());

	TestHotspotGridCallback cb2;
	grid.EnumerateHotspots(&cb2);
	assertIntegersEqual(0, cb2.GetCount());
}
