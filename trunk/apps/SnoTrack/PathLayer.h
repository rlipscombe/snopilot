#pragma once
#if defined(ENABLE_DESIGN_MODE)
#include "BspLayer.h"
#include "HotspotPath.h"
#include "CentreOfGravity.h"

class PathLayer : public BspLayer
{
	centres_t *m_centres;
	const HotspotPathCollection *m_paths;

public:
	PathLayer(const TCHAR *name, int weight, centres_t *centres, const HotspotPathCollection *paths)
		: BspLayer(name, weight, true, false), m_centres(centres), m_paths(paths)
	{
		ASSERT(m_paths);
	}

	virtual void OnRecalc(const CRect &logicalRange);

private:
	void InsertPath(const HotspotPath &path);
};
#endif /* ENABLE_DESIGN_MODE */
