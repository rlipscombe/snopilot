#pragma once
#include <map>

struct CentreOfGravity
{
	CentreOfGravity()
		: point(0,0), weight(0)
	{
	}

	CentreOfGravity(CPoint p, int w)
		: point(p), weight(w)
	{
	}

	CPoint point;
	int weight;
};

typedef std::map< int, CentreOfGravity > centres_t;
