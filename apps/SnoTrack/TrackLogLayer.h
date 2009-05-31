#pragma once
#include "BspLayer.h"
#include "PositionFix.h"

class TrackLogLayer : public BspLayer
{
	const PositionFixCollection *m_positions;

public:
	TrackLogLayer(const TCHAR *name, int weight, const PositionFixCollection *positions)
		: BspLayer(name, weight, true, false), m_positions(positions)
	{
	}

// Layer
public:
	virtual void OnRecalc(const CRect &logicalRange);
	virtual void OnSelect(const SelectionTimeSpanCollection &selection);

private:
	void FillBsp();

	void InsertLineDef(PositionFixCollection::const_iterator prev_fix, PositionFixCollection::const_iterator this_fix, ULONGLONG secondsSinceLast);
	using BspLayer::InsertLineDef;
};
