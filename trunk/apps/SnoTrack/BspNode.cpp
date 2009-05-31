#include "StdAfx.h"
#include "BspNode.h"

void BspNode::AssertValid() const
{
	// We've either got both children or neither child.
	ASSERT((m_topLeft && m_bottomRight) || (!m_topLeft && !m_bottomRight));

	// If we're not a leaf node, we're not allowed any items.
	ASSERT(IsLeafNode() || m_items.size() == 0);
}

// Why are RECTs endpoint-exclusive?
// http://blogs.msdn.com/oldnewthing/archive/2004/02/18/75652.aspx
void AssertLineInRange(const CRect &range, const LineDef &line)
{
	ASSERT(line.m_x1 >= range.left);
	ASSERT(line.m_x1 <= range.right);
	ASSERT(line.m_y1 >= range.top);
	ASSERT(line.m_y1 <= range.bottom);

	ASSERT(line.m_x2 >= range.left);
	ASSERT(line.m_x2 <= range.right);
	ASSERT(line.m_y2 >= range.top);
	ASSERT(line.m_y2 <= range.bottom);
}

int BspNode::GetTotalItemCount() const
{
	AssertValid();

	if (IsLeafNode())
		return GetItemCount();
	else
		return GetTopLeft()->GetTotalItemCount() + GetBottomRight()->GetTotalItemCount();
}

/* static */
BspNode *BspNode::VerticalSplit(const CRect &range, int depth)
{
	if (depth == 0)
		return NEW BspNode(range);

	// Divide the range with a vertical split, and then create the relevant nodes.
	int split = (range.left + range.right) / 2;
	CRect lhs(range.left, range.top, split, range.bottom);
	CRect rhs(split, range.top, range.right, range.bottom);

//	TRACE("VerticalSplit: (%d, %d, %d, %d) -> (%d, %d, %d, %d) : (%d, %d, %d, %d)\n",
//		range.left, range.top, range.right, range.bottom,
//		lhs.left, lhs.top, lhs.right, lhs.bottom,
//		rhs.left, rhs.top, rhs.right, rhs.bottom);

	BspNode *leftNode = HorizontalSplit(lhs, depth-1);
	BspNode *rightNode = HorizontalSplit(rhs, depth-1);

	return NEW BspNode(range, split, leftNode, rightNode);
}

/* static */
BspNode *BspNode::HorizontalSplit(const CRect &range, int depth)
{
	if (depth == 0)
		return NEW BspNode(range);

	// Divide the range with a vertical split, and then create the relevant nodes.
	int split = (range.top + range.bottom) / 2;
	CRect top(range.left, range.top, range.right, split);
	CRect bottom(range.left, split, range.right, range.bottom);

//	TRACE("HorizontalSplit: (%d, %d, %d, %d) -> (%d, %d, %d, %d) : (%d, %d, %d, %d)\n",
//		range.left, range.top, range.right, range.bottom,
//		top.left, top.top, top.right, top.bottom,
//		bottom.left, bottom.top, bottom.right, bottom.bottom);

	BspNode *topNode = VerticalSplit(top, depth-1);
	BspNode *bottomNode = VerticalSplit(bottom, depth-1);

	return NEW BspNode(range, split, topNode, bottomNode);
}

/* static */
BspNode *BspNode::Create(const CRect &range, int depth)
{
	return VerticalSplit(range, depth);
}

/** Work out where the line intersects the split.
 * See "Computer Graphics", F.S.Hill Jr, Prentice Hall, p75 for the details.
 */
int FindVerticalIntersection(const LineDef &line, int split)
{
	int yNew = line.m_y2 - MulDiv(line.m_x2 - split, line.m_y2 - line.m_y1, line.m_x2 - line.m_x1);
	return yNew;
}

int FindHorizontalIntersection(const LineDef &line, int split)
{
	int xNew = line.m_x2 - MulDiv(line.m_y2 - split, line.m_x2 - line.m_x1, line.m_y2 - line.m_y1);
	return xNew;
}

void BspNode::VerticalInsert(LineDef line)
{
	// This step is much easier if we ensure that the line goes from left to right.
	if (line.m_x1 > line.m_x2)
	{
		std::swap(line.m_x1, line.m_x2);
		std::swap(line.m_y1, line.m_y2);
	}

	// Attempting to insert a line that doesn't belong in this piece of the BSP?
//	AssertLineInRange(GetRange(), line);

//	TRACE("VerticalInsert: split = %d, range = (%d, %d, %d, %d), line = (%d, %d, %d, %d)\n",
//		m_split,
//		m_range.left, m_range.top, m_range.right, m_range.bottom,
//		line.m_x1, line.m_y1, line.m_x2, line.m_y2);

	// Does this node have any children?
	if (IsLeafNode())
	{
		InsertItem(line);
	}
	else
	{
		// We can save a couple of comparisons if we know that this is true.
		ASSERT(line.m_x1 <= line.m_x2);

		int split = GetSplit();

		if (line.m_x2 < split)
		{
			// The line fits entirely in the LHS.
			GetTopLeft()->HorizontalInsert(line);
		}
		else if (line.m_x1 >= split)
		{
			// The line fits entirely in the RHS.
			GetBottomRight()->HorizontalInsert(line);
		}
		else
		{
			// The line straddles the split.  It needs to be divided into two lines,
			// and they both need to be added.
			int yNew = FindVerticalIntersection(line, split);
			GetTopLeft()->HorizontalInsert(LineDef(line.m_x1, line.m_y1, split, yNew, line.m_cr, line.m_width, line.m_timestamp, line.m_gap));
			GetBottomRight()->HorizontalInsert(LineDef(split, yNew, line.m_x2, line.m_y2, line.m_cr, line.m_width, line.m_timestamp, line.m_gap));
		}
	}
}

void BspNode::HorizontalInsert(LineDef line)
{
	// This is much easier if we ensure the line goes from small-y to big-y.
	if (line.m_y1 > line.m_y2)
	{
		std::swap(line.m_x1, line.m_x2);
		std::swap(line.m_y1, line.m_y2);
	}

	// Attempting to insert a line that doesn't belong in this piece of the BSP?
//	AssertLineInRange(GetRange(), line);

//	TRACE("HorizontalInsert: split = %d, range = (%d, %d, %d, %d), line = (%d, %d, %d, %d)\n",
//		m_split,
//		m_range.left, m_range.top, m_range.right, m_range.bottom,
//		line.m_x1, line.m_y1, line.m_x2, line.m_y2);

	if (IsLeafNode())
	{
		InsertItem(line);
	}
	else
	{
		// We can save a couple of comparisons if we know that this is true.
		ASSERT(line.m_y1 <= line.m_y2);

		int split = GetSplit();

		// This split is horizontal, so does the line go above or below the line?
		// There are three cases:
		if (line.m_y2 < split)
		{
			// The line fits entirely in the top.
			GetTopLeft()->VerticalInsert(line);
		}
		else if (line.m_y1 >= split)
		{
			// The line fits entirely in the bottom.
			GetBottomRight()->VerticalInsert(line);
		}
		else
		{
			// The line straddles the split.  It needs to be divided into two lines,
			// and they both need to be added.
			int xNew = FindHorizontalIntersection(line, split);
			GetTopLeft()->VerticalInsert(LineDef(line.m_x1, line.m_y1, xNew, split, line.m_cr, line.m_width, line.m_timestamp, line.m_gap));
			GetBottomRight()->VerticalInsert(LineDef(xNew, split, line.m_x2, line.m_y2, line.m_cr, line.m_width, line.m_timestamp, line.m_gap));
		}
	}
}

bool BspNode::LineIntersectsRange(CPoint p1, CPoint p2, const CRect &range)
{
	// If both ends of the line are to the left, it's not.
	// Same if they're both above, below or to the right.
	if (p1.x < range.left && p2.x < range.left)
		return false;
	if (p1.x > range.right && p2.x > range.right)
		return false;
	if (p1.y < range.top && p2.y < range.top)
		return false;
	if (p1.y > range.bottom && p2.y > range.bottom)
		return false;

	// Otherwise, it does intersect the range.
	return true;
}

bool BspNode::LineIntersectsRange(const LineDef &line, const CRect &range)
{
	CPoint p1(line.m_x1, line.m_y1);
	CPoint p2(line.m_x2, line.m_y2);

	return LineIntersectsRange(p1, p2, range);
}

void BspNode::Insert(const LineDef &line)
{
	// Do some simple filtering.  Is this line actually in the BSP?
	if (!LineIntersectsRange(line, m_range))
		return;

	VerticalInsert(line);
}

void BspNode::VerticalSearch(const Rectangle &r, BspSearchCallback *pCallback)
{
	AssertValid();

	if (IsLeafNode())
	{
		pCallback->OnLeafNode(m_range, m_items);
	}
	else
	{
		ASSERT(m_topLeft && m_bottomRight);

		int split = GetSplit();

		bool go_top_left = ((r.a.x < split) || (r.b.x < split) || (r.c.x < split) || (r.d.x < split));
		bool go_bottom_right = ((r.a.x > split) || (r.b.x > split) || (r.c.x > split) || (r.d.x > split));

		// The partitioning shouldn't leave any gaps, and we shouldn't have taken a wrong turning.
		// So at least one direction should be valid.
		ASSERT(go_top_left || go_bottom_right);

		if (go_top_left)
			GetTopLeft()->HorizontalSearch(r, pCallback);

		if (go_bottom_right)
			GetBottomRight()->HorizontalSearch(r, pCallback);
	}
}

void BspNode::HorizontalSearch(const BspNode::Rectangle &r, BspSearchCallback *pCallback)
{
	AssertValid();

	if (IsLeafNode())
	{
		pCallback->OnLeafNode(m_range, m_items);
	}
	else
	{
		ASSERT(m_topLeft && m_bottomRight);

		int split = GetSplit();

		bool go_top_left = ((r.a.y < split) || (r.b.y < split) || (r.c.y < split) || (r.d.y < split));
		bool go_bottom_right = ((r.a.y > split) || (r.b.y > split) || (r.c.y > split) || (r.d.y > split));

		// The partitioning shouldn't leave any gaps, and we shouldn't have taken a wrong turning.
		// So at least one direction should be valid.
		ASSERT(go_top_left || go_bottom_right);

		if (go_top_left)
			GetTopLeft()->VerticalSearch(r, pCallback);

		if (go_bottom_right)
			GetBottomRight()->VerticalSearch(r, pCallback);
	}
}

void BspNode::Search(const BspNode::Rectangle &r, BspSearchCallback *pCallback)
{
	VerticalSearch(r, pCallback);
}

void BspNode::Search(BspSearchCallback *pCallback)
{
	Rectangle everything(GetRange());
	Search(everything, pCallback);
}

void BspNode::Walk(BspWalkCallback *pCallback)
{
	if (IsLeafNode())
	{
		pCallback->OnLeafNode(&m_items);
	}
	else
	{
		m_topLeft->Walk(pCallback);
		m_bottomRight->Walk(pCallback);
	}
}
