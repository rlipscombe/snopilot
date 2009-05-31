#pragma once
#include "Layer.h"

struct GridBounds;
class DrawSite;

class GridLinesLayer : public Layer
{
public:
	GridLinesLayer(const TCHAR *name, int weight)
		: Layer(name, weight, true, false)
	{
	}

// Layer
public:
	virtual void OnDraw(DrawSite *drawSite, CDC *pDC);

private:
	double GetLongitudeSpacing() const { return 1.0/60.0; /* Every minute of arc */ }
	double GetLatitudeSpacing() const { return 1.0/60.0; /* Every minute of arc */ }

	void DrawLongitudeLines(DrawSite *drawSite, CDC *pDC, GridBounds bounds);
	void DrawLongitudeLine(DrawSite *drawSite, CDC *pDC, double lon, double minLat, double maxLat);
	void DrawLongitudeLabels(DrawSite *drawSite, CDC *pDC, double lon, CPoint p0, CPoint p1);

	void DrawLatitudeLines(DrawSite *drawSite, CDC *pDC, GridBounds bounds);
	void DrawLatitudeLine(DrawSite *drawSite, CDC *pDC, double lat, double minLon, double maxLon);
	void DrawLatitudeLabels(DrawSite *drawSite, CDC *pDC, double lat, CPoint p0, CPoint p1);
};

