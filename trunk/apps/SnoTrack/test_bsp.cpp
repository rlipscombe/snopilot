#include "StdAfx.h"
#include "BspNode.h"
#include "test/test_assert.h"
#include "image/save_dib.h"
#include <algorithm>

void test_CreateBSP()
{
	// There are a couple of different options for creating the BSP.  We could
	// divide up the space so that, at each split, the number of items in each half is
	// roughly balanced.
	// This basically requires sweeping a line across the space until roughly half of the
	// data points are in each half.
	//
	// It's not too bad -- we just sort the data points by X coordinate (for a vertical split,
	// giving LHS and RHS).
	//
	// It ought to come out quite quickly, too, since we already divided the data points
	// when we sorted them.  It's a nice recursive algorithm, I guess.
	//
	// Another alternative is to simply divide the space in half.  This could lead to more points
	// in one half than the other, but it's simpler.  It's also cheaper to update.  If points
	// are added, we can just find out which node they ought to be in, and add them.  In the other
	// version, we'd need to "rebalance" the BSP.
	//
	// The first version is slightly complicated when dealing with lines.  How do we balance it so
	// that the number of lines in each half is roughly the same?  The simple answer here is
	// that we don't bother -- we'll just examine the line starting point to decide where
	// a line goes -- we'll have to split it into two lines at some point in the future, anyway.

	// The first simple test has us dividing up the space.
	CRect range(0, 0, 100, 100);

	// BLOCK:
	{
		// With a depth of zero, everything should end up in the root node.
		BspNode *bsp = BspNode::Create(range, 0);
		assertIntegersEqual(1, bsp->GetLeafCount());
		
		// We should have a root node.
		assertTrue(bsp != NULL);

		// It should cover the entire range.
		assertTrue(bsp->GetRange() == range);

		// It should have no children.
		assertTrue(bsp->IsLeafNode());
		assertFalse(bsp->GetTopLeft());
		assertFalse(bsp->GetBottomRight());

		delete bsp;
	}

	// BLOCK:
	{
		// With a depth of one, we should end up with a left side and a right side,
		// assuming that the first split is vertical.
		BspNode *bsp = BspNode::Create(range, 1);
		assertIntegersEqual(2, bsp->GetLeafCount());

		// We should have a root node.
		assertTrue(bsp != NULL);

		// It should cover the entire range.
		assertTrue(bsp->GetRange() == range);

		// It ought to have children.
		assertFalse(bsp->IsLeafNode());
		assertTrue(bsp->GetTopLeft());
		assertTrue(bsp->GetBottomRight());

		// They ought to cover the relevant pieces of the range.
		assertTrue(bsp->GetTopLeft()->GetRange() == CRect(0, 0, 50, 100));
		assertTrue(bsp->GetBottomRight()->GetRange() == CRect(50, 0, 100, 100));

		delete bsp;
	}

	// BLOCK:
	{
		// With a depth of two, we should end up with a left side and a right side,
		// each of which should be divided into a top half and a bottom half.
		BspNode *bsp = BspNode::Create(range, 2);
		assertIntegersEqual(4, bsp->GetLeafCount());

		// We should have a root node.
		assertTrue(bsp != NULL);

		// It should cover the entire range.
		assertTrue(bsp->GetRange() == range);

		// It ought to have children.
		assertFalse(bsp->IsLeafNode());
		assertTrue(bsp->GetTopLeft());
		assertTrue(bsp->GetBottomRight());

		// They ought to cover the relevant pieces of the range.
		assertTrue(bsp->GetTopLeft()->GetRange() == CRect(0, 0, 50, 100));
		assertTrue(bsp->GetBottomRight()->GetRange() == CRect(50, 0, 100, 100));

		// The left side ought to have children.
		assertFalse(bsp->GetTopLeft()->IsLeafNode());
		assertTrue(bsp->GetTopLeft()->GetTopLeft());
		assertTrue(bsp->GetTopLeft()->GetBottomRight());

		// They ought to cover the relevant pieces of the range.
		assertTrue(bsp->GetTopLeft()->GetTopLeft()->GetRange() == CRect(0, 0, 50, 50));
		assertTrue(bsp->GetTopLeft()->GetBottomRight()->GetRange() == CRect(0, 50, 50, 100));

		// The right side ought to have children.
		assertFalse(bsp->GetBottomRight()->IsLeafNode());
		assertTrue(bsp->GetBottomRight()->GetTopLeft());
		assertTrue(bsp->GetBottomRight()->GetBottomRight());

		// Check that these, too, cover the relevant pieces.
		assertTrue(bsp->GetBottomRight()->GetTopLeft()->GetRange() == CRect(50, 0, 100, 50));
		assertTrue(bsp->GetBottomRight()->GetBottomRight()->GetRange() == CRect(50, 50, 100, 100));

		delete bsp;
	}
}

class TestRectDrawCallback : public BspSearchCallback
{
	CDC *m_pDC;

public:
	TestRectDrawCallback(CDC *pDC)
		: m_pDC(pDC)
	{
	}

public:
	virtual void OnLeafNode(const CRect &range, const items_t &items)
	{
		m_pDC->Rectangle(range.left, range.top, range.right, range.bottom);
	}
};

class TestLineDrawCallback : public BspSearchCallback
{
	CDC *m_pDC;

public:
	TestLineDrawCallback(CDC *pDC)
		: m_pDC(pDC)
	{
	}

public:
	virtual void OnLeafNode(const CRect &range, const items_t &items)
	{
		for (items_t::const_iterator i = items.begin(); i != items.end(); ++i)
		{
			m_pDC->MoveTo(i->m_x1, i->m_y1);
			m_pDC->LineTo(i->m_x2, i->m_y2);
		}
	}
};

void DrawBSPToFile(BspNode *bsp, const CRect &range, const _TCHAR *pszFileName)
{
	// There's no code in here to cope with ranges that don't start at (0,0)
	ASSERT(range.left == 0 && range.top == 0);

	CWindowDC screenDC(NULL);

	CDC memoryDC;
	VERIFY(memoryDC.CreateCompatibleDC(&screenDC));

	// We need a bitmap of some size or other.
	CBitmap memoryBitmap;
	memoryBitmap.CreateCompatibleBitmap(&screenDC, range.Width(), range.Height());

	CBitmap *oldMemoryBitmap = memoryDC.SelectObject(&memoryBitmap);

	memoryDC.PatBlt(0, 0, range.Width(), range.Height(), WHITENESS);

//	TestRectDrawCallback drawRectangles(&memoryDC);
//	bsp->Search(BspNode::Rectangle(range), &drawRectangles);
	TestLineDrawCallback drawLines(&memoryDC);
	bsp->Search(BspNode::Rectangle(range), &drawLines);

	// Save the DIB to disk.
	SaveDib(memoryDC, pszFileName, true);

	memoryDC.SelectObject(oldMemoryBitmap);
}

// Now, when we're inserting items in the BSP, we travel down the tree, deciding
// which side of the split they would go.  Having decided on that, we can attach them
// to the leaf node at the end of the chain.  Now, if we're going to make this generic,
// we need to have some generic container at the end of the chain.
// Typically, in C, we'd use a void * which would point at our container.  Our wrapper
// class would then know that this was a vector<Point> or vector<Line>.
// We'd also need something to make the decision about which side of the split something
// was to go.
// If we were templating this, it's not such a big deal.  We'd just have something like this:
// 
// template <typename T, class LorR, class TorB>
//   class BinarySpacePartition { /* ... */ };
//
// This doesn't work so well in the face of lines, though, which might need to be divided
// if the line crosses the split.  Mind you, I can't see how that's going to work
// anyway.
//
// OK, so when we insert a line into the BSP, we have to decide, at each level, whether
// it's to go on one side of the split or the other.  Now, if we decide that
// it's got to go on both, we need to split it and continue recursing.
//
// So, if we have a comparison function that returns <0 for LHS, >0 for RHS and ==0 for both,
// we then need to ask another function to split the line for us.  Is that so hard?
//
// What if it's not lines, but (filled) polygons, for example?  We'd need to discover that
// the polygon straddled the line, and split it in two.  We'd then recurse with the
// two halves.  Maybe we'd tie them together, but that's not the BSP's problem, I'm thinking.

void test_InsertLineBSP_Depth0()
{
	CRect range(0, 0, 100, 100);

	// We'll start with a depth zero BSP -- i.e. just the one node.  The line should end up in it.
	BspNode *bsp = BspNode::Create(range, 0);

	LineDef line1(30, 45, 45, 30, RGB(0,0,0), 1);
	bsp->Insert(line1);

	assertIntegersEqual(1, bsp->GetItemCount());
	delete bsp;
}

void test_InsertLineBSP_Depth1()
{
	CRect range(0, 0, 100, 100);

	// Depth 1 -- i.e. there's just the LHS and RHS.
	BspNode *bsp = BspNode::Create(range, 1);
	assertIntegersEqual(2, bsp->GetLeafCount());

	// If we put a single line in it, that will fit completely in a square, we should be
	// able to find it again.
	LineDef line1(30, 45, 45, 30, RGB(0,0,0), 1);

	bsp->Insert(line1);

	// The relevant node ought to have one item in it.
	assertIntegersEqual(1, bsp->GetTopLeft()->GetItemCount());

	// Let's add an item that straddles the two sides.
	bsp->Insert(LineDef(30, 70, 70, 55, RGB(0,0,0), 1));

	// Let's add another item (actually the same line) that straddles the two sides, but goes the other way.
	bsp->Insert(LineDef(70, 55, 30, 70, RGB(0,0,0), 1));

	delete bsp;
}

void test_InsertLineBSP_Depth4()
{
	CRect range(0, 0, 100, 100);

	// We've got a BSP, depth 4 -- i.e. there's 16 squares in it.
	BspNode *bsp = BspNode::Create(range, 4);
	assertIntegersEqual(16, bsp->GetLeafCount());

	// If we put a single line in it, that will fit completely in a square, we should be
	// able to find it again.
	bsp->Insert(LineDef(30, 45, 45, 30, RGB(0,0,0), 1));

	// The relevant node ought to have one item in it.
	assertIntegersEqual(1, bsp->GetTopLeft()->GetTopLeft()->GetBottomRight()->GetBottomRight()->GetItemCount());

	// Put another line in it.  This one goes in a different square.
	bsp->Insert(LineDef(85, 55, 85, 70, RGB(0,0,0), 1));	// This one's vertical.
	
	// The relevant node ought to have another item in it.
	assertIntegersEqual(1, bsp->GetBottomRight()->GetBottomRight()->GetBottomRight()->GetTopLeft()->GetItemCount());

	// Put another line in the same square as the first one.
	bsp->Insert(LineDef(40, 40, 45, 45, RGB(0,0,0), 1));
	assertIntegersEqual(2, bsp->GetTopLeft()->GetTopLeft()->GetBottomRight()->GetBottomRight()->GetItemCount());

	// A line that starts exactly on the line, and is horizontal.
	bsp->Insert(LineDef(75, 90, 85, 90, RGB(0,0,0), 1));
	assertIntegersEqual(1, bsp->GetBottomRight()->GetBottomRight()->GetBottomRight()->GetBottomRight()->GetItemCount());

	// A line that is against the edge of a square, and is vertical.
	bsp->Insert(LineDef(75, 90, 80, 90, RGB(0,0,0), 1));
	assertIntegersEqual(2, bsp->GetBottomRight()->GetBottomRight()->GetBottomRight()->GetBottomRight()->GetItemCount());

	// There should be 5 items somewhere in the BSP.
	assertIntegersEqual(5, bsp->GetTotalItemCount());

	// Let's add some that straddle squares.  These are the two simple ones from above.
	bsp->Insert(LineDef(30, 70, 70, 55, RGB(0,0,0), 1));
	bsp->Insert(LineDef(70, 55, 30, 70, RGB(0,0,0), 1));

	assertIntegersEqual(2, bsp->GetTopLeft()->GetBottomRight()->GetBottomRight()->GetTopLeft()->GetItemCount());
	assertIntegersEqual(2, bsp->GetBottomRight()->GetBottomRight()->GetTopLeft()->GetTopLeft()->GetItemCount());

	// Here's one that straddles a square vertically.
	bsp->Insert(LineDef(5, 45, 20, 60, RGB(0,0,0), 1));

	// Another one, different square -- expressed the other way round.
	bsp->Insert(LineDef(90, 30, 85, 20, RGB(0,0,0), 1));

	// A horizontal one that straddles two squares.
	bsp->Insert(LineDef(20, 90, 45, 90, RGB(0,0,0), 1));

	// A horizontal one that runs along the edge of a square.  This requires three lines (+- 1)
	bsp->Insert(LineDef(55, 25, 70, 25, RGB(0,0,0), 1));
	bsp->Insert(LineDef(55, 24, 70, 24, RGB(0,0,0), 1));
	bsp->Insert(LineDef(55, 26, 70, 26, RGB(0,0,0), 1));

	// A longer horizontal one that runs along the edge of a couple of squares.  This requires three lines (+- 1)
	bsp->Insert(LineDef(45, 75, 95, 75, RGB(0,0,0), 1));
	bsp->Insert(LineDef(45, 74, 95, 74, RGB(0,0,0), 1));
	bsp->Insert(LineDef(45, 76, 95, 76, RGB(0,0,0), 1));

	// A vertical one that straddles two squares.
	// A horizontal one that straddles more than two squares.
	// A vertical one that straddles more than two squares.
	// A diagonal one that straddles more than two squares.

	// A vertical one that runs along the edge of a couple of squares.
	bsp->Insert(LineDef(75, 20, 75, 60, RGB(0,0,0), 1));
	bsp->Insert(LineDef(74, 20, 74, 60, RGB(0,0,0), 1));
	bsp->Insert(LineDef(76, 20, 76, 60, RGB(0,0,0), 1));

	// Some that traverse the whole space.
//	bsp->Insert(LineDef(0, 0, 100, 100, RGB(0,0,0), 1));
//	bsp->Insert(LineDef(100, 100, 0, 0, RGB(0,0,0), 1));
//	bsp->Insert(LineDef(0, 100, 100, 0, RGB(0,0,0), 1));
//	bsp->Insert(LineDef(100, 0, 0, 100, RGB(0,0,0), 1));

	// Try drawing that particular BSP.
	DrawBSPToFile(bsp, range, _T("test_InsertLineBSP_Depth4.bmp"));

	delete bsp;
}

class CollectLineDefs : public BspSearchCallback
{
	items_t *m_items;

public:
	CollectLineDefs(items_t *v)
		: m_items(v)
	{
	}

public:
	virtual void OnLeafNode(const CRect &rect, const items_t &items)
	{
		std::copy(items.begin(), items.end(), std::back_inserter(*m_items));
	}
};

// A line matches even if it goes the other way.
class MatchLineDef
{
	LineDef m_line;

public:
	MatchLineDef(const LineDef &line)
		: m_line(line)
	{
	}

	bool operator() (const LineDef &line) const
	{
		return (MatchPoint(m_line.m_x1, m_line.m_y1, line.m_x1, line.m_y1) && MatchPoint(m_line.m_x2, m_line.m_y2, line.m_x2, line.m_y2))
			|| (MatchPoint(m_line.m_x1, m_line.m_y1, line.m_x2, line.m_y2) && MatchPoint(m_line.m_x2, m_line.m_y2, line.m_x1, line.m_y1));
	}

	bool MatchPoint(int ax, int ay, int bx, int by) const
	{
		return (ax == bx) && (ay == by);
	}
};

void test_InsertLineBSP_VerticalLine2()
{
	CRect range(0, 0, 100, 100);
	BspNode *bsp = BspNode::Create(range, 2);

	// The bug that I found is that this would cause the
	// line to be split with the endpoints offset in the x direction.
	// This is wrong.
	bsp->Insert(LineDef(25, 10, 25, 90, RGB(0,0,0), 1));

	DrawBSPToFile(bsp, range, "test_InsertLineBSP_VerticalLine2.bmp");
	delete bsp;
}

void test_FindVerticalIntersection()
{
	// This line is horizontal.
	int y = 50;
	LineDef line(10, y, 90, y, RGB(0,0,0), 1);

	// Let's see where it intersects the line x = 50.
	int x = 50;

	int yNew = FindVerticalIntersection(line, x);

	// Since it's horizontal, the y-coordinate shouldn't have changed.
	assertIntegersEqual(y, yNew);
}

void test_FindHorizontalIntersection()
{
	// This line is vertical.
	int x = 50;
	LineDef line(x, 10, x, 90, RGB(0,0,0), 1);

	// Let's see where it intersects the line y = 50.
	int y = 50;

	int xNew = FindHorizontalIntersection(line, y);

	// Since it's vertical, the x-coordinate should not have changed.
	assertIntegersEqual(x, xNew);
}

void test_InsertLineBSP_VerticalLines()
{
	CRect range(0, 0, 100, 100);

	BspNode *bsp = BspNode::Create(range, 4);

	bsp->Insert(LineDef(75, 20, 75, 60, RGB(0,0,0), 1));
	bsp->Insert(LineDef(74, 20, 74, 60, RGB(0,0,0), 1));
	bsp->Insert(LineDef(76, 20, 76, 60, RGB(0,0,0), 1));

	DrawBSPToFile(bsp, range, _T("test_InsertLineBSP_VerticalLines.bmp"));

	delete bsp;
}

// Found this one with real data.
void test_InsertLineBSP_RealData()
{
	CRect range(-136883336, 64219977, -136789972, 64343324);
	LineDef line(-136874576, 64340741, -136846688, 64327427, RGB(0,0,0), 1);
	int depth = 12;

	BspNode *bsp = BspNode::Create(range, depth);

	// This caused an ASSERT in the code.
	bsp->Insert(line);

	delete bsp;
}

void test_InsertLineBSP()
{
	// Test some primitives.
	test_FindHorizontalIntersection();
	test_FindVerticalIntersection();

	// Some general testing.
	test_InsertLineBSP_Depth0();
	test_InsertLineBSP_Depth1();
	test_InsertLineBSP_Depth4();
	
	// Some more specific testing; usually because we found a bug with real data.
	test_InsertLineBSP_VerticalLine2();
	test_InsertLineBSP_VerticalLines();
	
	// This one's because of a particular bug we found with real data.
	test_InsertLineBSP_RealData();
}

// A balanced BSP would work like a balanced tree (B-Tree).  We'd start with no root node (or an empty root node).
// We'd then insert an item into it.  Now, if the item fills the node, we'd have to split the node.  When we split
// the node, we need to decide whether this is going to be a vertical split or a horizontal split.
// Now, for the first node, it doesn't much matter, so we can just assume that it's a vertical split, giving us
// LHS and RHS.
//
// So, we'd take the items in the node, and sort them by x coordinate.  Then we'd create a pair of new nodes,
// each containing half of them.  The original node would be replaced with a parent node, containing a pointer
// to the two new nodes.  So, it's not quite the same as a proper B-Tree, because we've not got a sufficiently
// high fan-out on the parent node.  Can we make it work?
//
// Even in this not-quite-B-Tree state, though, it does have the feature that each node contains roughly the
// same number of items.  It ought to naturally end up balanced, too.  This may not prove to be very useful.

class TestCallback : public BspSearchCallback
{
	int m_squaresVisited;
	size_t m_lineDefCount;

public:
	TestCallback()
		: m_squaresVisited(0), m_lineDefCount(0)
	{
	}

	int GetSquaresVisited() const { return m_squaresVisited; }
	int GetLineDefCount() const { return (int)m_lineDefCount; }

// BspSearchCallback
public:
	virtual void OnLeafNode(const CRect &rect, const items_t &items)
	{
		++m_squaresVisited;
		m_lineDefCount += items.size();
	}
};

void test_SearchBSP()
{
	CRect range(0, 0, 100, 100);

	BspNode *bsp = BspNode::Create(range, 4);

	// Put a line into the BSP.  Then we'll see if we're told to draw it later.
	bsp->Insert(LineDef(30, 45, 45, 30, RGB(0,0,0), 1));

	// The first test has us miss it completely.
	{
		TestCallback cb;
		BspNode::Rectangle r1(CPoint(55, 60), CPoint(70, 60), CPoint(70, 70), CPoint(55, 70));
		bsp->Search(r1, &cb);
		assertIntegersEqual(1, cb.GetSquaresVisited());
		assertIntegersEqual(0, cb.GetLineDefCount());
	}

	// The second test has us miss it, but using a rotated rectangle.
	// Note that it doesn't have to be a strict rectangle.
	{
		TestCallback cb;
		BspNode::Rectangle r2(CPoint(60, 10), CPoint(80, 30), CPoint(70, 40), CPoint(50, 20));
		bsp->Search(r2, &cb);
		assertIntegersEqual(4, cb.GetSquaresVisited());
		assertIntegersEqual(0, cb.GetLineDefCount());
	}

	// The third test completely encloses the square.
	{
		TestCallback cb;
		BspNode::Rectangle r3(CPoint(25, 25), CPoint(50, 25), CPoint(50, 50), CPoint(25, 50));
		bsp->Search(r3, &cb);

		// Should have found the line this time.

		assertIntegersEqual(1, cb.GetSquaresVisited());
		assertIntegersEqual(1, cb.GetLineDefCount());
	}

	// The 4th test has us use a larger rectangle that intersects
	// with the line, but also some other squares.
	{
		TestCallback cb;
		BspNode::Rectangle r4(CPoint(30, 35), CPoint(45, 60), CPoint(25, 75), CPoint(10, 50));
		bsp->Search(r4, &cb);

		// Should have found the line this time.
		assertIntegersEqual(4, cb.GetSquaresVisited());
		assertIntegersEqual(1, cb.GetLineDefCount());
	}

	delete bsp;
}
