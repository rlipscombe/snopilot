#pragma once

/** Move 'inner' so that it's centered within 'outer'.
 * Return the new rectangle.
 */
inline CRect CenterRectangle(const CRect &outer, const CRect &inner)
{
    CRect result(inner);
    result.OffsetRect(outer.CenterPoint() - inner.CenterPoint());
    return result;
}

/** Move 'inner' so that it's centered within 'outer'.
 * Return the new rectangle.
 */
inline CRect CenterRectangle(const CRect &outer, const CSize &inner)
{
	return CenterRectangle(outer, CRect(CPoint(0, 0), inner));
}
