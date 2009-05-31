#pragma once
#include <vector>
#include "utf8.h"

class HotspotPath : public std::vector< int >
{
	utf8_string m_pathName;
	double m_distance;
	bool m_isLift;
	int m_imageIndex;
	
public:
	HotspotPath(const utf8_string &pathName, bool isLift, int imageIndex)
		: m_pathName(pathName), m_isLift(isLift), m_imageIndex(imageIndex)
	{
	}

	void SetName(const utf8_string &name) { m_pathName = name; }
	utf8_string GetName() const { return m_pathName; }

	void SetImageIndex(int imageIndex) { m_imageIndex = imageIndex; }
	int GetImageIndex() const { return m_imageIndex; }

	void SetDistance(double distance) { m_distance = distance; }
	double GetDistance() const { return m_distance; }

	void SetLift(bool b) { m_isLift = b; }
	bool IsLift() const { return m_isLift; }
};

typedef std::vector<HotspotPath> HotspotPathCollection;
