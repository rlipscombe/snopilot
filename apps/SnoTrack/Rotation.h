#pragma once

/** This represents a rotation value.  It saves recalculating some stuff.
 */
struct Rotation
{
	double theta;		///< In radians.
	double sin_theta;	///< sin(theta): it should be recalculated every time you change theta.
	double cos_theta;	///< cos(theta): it should be recalculated every time you change theta.

	Rotation()
		: theta(0), sin_theta(0), cos_theta(1)
	{
	}

	Rotation(double t, double s, double c)
		: theta(t), sin_theta(s), cos_theta(c)
	{
	}

	CPoint Forward(CPoint p)
	{
		double x = (p.x * cos_theta) - (p.y * sin_theta);
		double y = (p.x * sin_theta) + (p.y * cos_theta);

		return CPoint((LONG)x, (LONG)y);
	}

	CPoint Reverse(CPoint p)
	{
		// We can use the following identities:
		// cos(-t) === cos(t); sin(-t) === -sin(t)
		double x = (p.x * cos_theta) - (p.y * -sin_theta);
		double y = (p.x * -sin_theta) + (p.y * cos_theta);

		return CPoint((LONG)x, (LONG)y);
	}
};
