#pragma once

struct GridBounds;
struct Rotation;

class DrawSite
{
public:
	virtual CRect GetInvalidRect() const = 0;
	virtual CRect GetClientRect() const = 0;

	/** Get the min/max lat/lon of the data set.
	 * Useful for drawing a boundary or grid lines.
	 */
	virtual GridBounds GetBounds() = 0;

	/** Get the scale of the view. */
	virtual int GetScaleFactor() = 0;

	/** Get the rotation values for the view. */
	virtual Rotation GetRotation() = 0;

	/** Given (lat,lon) coordinates on the 'sphere', map it to client coordinates.
	 */
	virtual CPoint GetClientPointFromSphere(double lon, double lat) = 0;

	virtual CRect GetLogicalRange() = 0;
	virtual CPoint GetClientPointFromLogicalPoint(CPoint logical) = 0;
	virtual CPoint GetLogicalPointFromClientPoint(CPoint client) = 0;
	CPoint GetClientPointFromLogicalPoint(int x, int y) { return GetClientPointFromLogicalPoint(CPoint(x, y)); }

	virtual CRect GetDeviceRange() = 0;
	virtual CPoint GetClientPointFromDevicePoint(CPoint device) = 0;
	CPoint GetClientPointFromDevicePoint(int x, int y) { return GetClientPointFromDevicePoint(CPoint(x, y)); }
};
