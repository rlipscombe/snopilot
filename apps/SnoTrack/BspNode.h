#pragma once
#include <vector>
#include "LineDef.h"

class BspSearchCallback;
class BspWalkCallback;

class BspNode
{
	// Every BSP node covers a range.
	CRect m_range;

	// Internal nodes have pointers to their child nodes.
	BspNode *m_topLeft;
	BspNode *m_bottomRight;

	// Internal nodes also have a split.  This is the x- or y-coordinate where the split occurred.
	int m_split;

	// Leaf nodes have items in them.
	typedef std::vector< LineDef > items_t;
	items_t m_items;

private:
	BspNode(const CRect &range)
		: m_range(range), m_split(0), m_topLeft(NULL), m_bottomRight(NULL)
	{
		// A quick stab at how many LineDefs are in this node.
		// It'll save a little bit of time in malloc().
		m_items.reserve(50);
	}

	BspNode(const CRect &range, int split, BspNode *topLeft, BspNode *bottomRight)
		: m_range(range), m_split(split), m_topLeft(topLeft), m_bottomRight(bottomRight)
	{
		// If we've got one child, we need both.
		ASSERT(m_topLeft && m_bottomRight);
	}

public:
	static BspNode *Create(const CRect &range, int depth);

	~BspNode()
	{
		delete m_topLeft;
		delete m_bottomRight;
	}

private:
	static BspNode *VerticalSplit(const CRect &range, int depth);
	static BspNode *HorizontalSplit(const CRect &range, int depth);

public:
	CRect GetRange() { return m_range; }

// Internal node details
public:
	BspNode *GetTopLeft() { return m_topLeft; }
	BspNode *GetBottomRight() { return m_bottomRight; }

	const BspNode *GetTopLeft() const { return m_topLeft; }
	const BspNode *GetBottomRight() const { return m_bottomRight; }

	int GetSplit() const { return m_split; }

	int GetLeafCount() const
	{
		AssertValid();

		// If we've got no children, then it's just one.
		if (m_topLeft)
			return m_topLeft->GetLeafCount() + m_bottomRight->GetLeafCount();
		else
			return 1;
	}

// Leaf node details
public:
	bool IsLeafNode() const { return !m_topLeft; }

	int GetItemCount() const
	{
		AssertValid();

		return (int)m_items.size();
	}

	int GetTotalItemCount() const;

// Putting things in the BSP.
public:
	void Insert(const LineDef &line);

private:
	bool LineIntersectsRange(CPoint p1, CPoint p2, const CRect &range);
	bool LineIntersectsRange(const LineDef &line, const CRect &range);

	void VerticalInsert(LineDef line);
	void HorizontalInsert(LineDef line);

	void InsertItem(const LineDef &line)
	{
		AssertValid();

		m_items.push_back(line);
	}

// Drawing the BSP
public:
	/** Not a Win32 RECT, because it's not necessarily
	 * aligned with the horizontal/vertical axes.
	 * In fact, it's not necessarily a strict rectangle, because
	 * it could be a bit wonky.
	 */
	struct Rectangle
	{
		CPoint a, b, c, d;

		Rectangle(CPoint aa, CPoint bb, CPoint cc, CPoint dd)
			: a(aa), b(bb), c(cc), d(dd)
		{
		}

		Rectangle(const CRect &rect)
			: a(rect.left, rect.top), b(rect.right, rect.top), c(rect.right, rect.bottom), d(rect.left, rect.bottom)
		{
		}

	private:
		// This constructor isn't implemented.  It's because CPoint has an implicit
		// int->CPoint conversion, and we'd rather not use it by accident.
		Rectangle(int a, int b, int c, int d);
	};

	void Search(BspSearchCallback *pCallback);
	void Search(const Rectangle &r, BspSearchCallback *pCallback);
	void Walk(BspWalkCallback *pCallback);

private:
	void VerticalSearch(const BspNode::Rectangle &r, BspSearchCallback *pCallback);
	void HorizontalSearch(const BspNode::Rectangle &r, BspSearchCallback *pCallback);

private:
	void AssertValid() const;
};

class BspSearchCallback
{
public:
	typedef std::vector< LineDef > items_t;

	virtual void OnLeafNode(const CRect &rect, const items_t &items) = 0;
};

class BspWalkCallback
{
public:
	typedef std::vector< LineDef > items_t;

	virtual void OnLeafNode(items_t *items) = 0;
};

int FindVerticalIntersection(const LineDef &line, int split);
int FindHorizontalIntersection(const LineDef &line, int split);
