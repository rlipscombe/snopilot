#pragma once

class Projection
{
	PJ *m_proj;
	static const int CARTESIAN_SCALAR = 10;

public:
	Projection()
		: m_proj(NULL)
	{
	}

	~Projection()
	{
		if (m_proj)
			pj_free(m_proj);
	}

	bool Create(int pargc, char *pargv[])
	{
		ASSERT(!m_proj);
		m_proj = pj_init(pargc, pargv);

		if (!m_proj)
			return false;

		return true;
	}

	CPoint ForwardDeg(double lon_deg, double lat_deg) const
	{
		projUV in;
		in.u = DegreesToRadians(lon_deg);
		in.v = DegreesToRadians(lat_deg);

		projUV out = pj_fwd(in, m_proj);

		int x = (int)(out.u * CARTESIAN_SCALAR);
		int y = (int)(out.v * CARTESIAN_SCALAR);

		return CPoint(x, y);
	}

	projUV ReverseDeg(CPoint p) const
	{
		projUV out = ReverseRad(p);

		out.u = RadiansToDegrees(out.u);
		out.v = RadiansToDegrees(out.v);

		return out;
	}

	projUV ReverseRad(CPoint p) const
	{
		projUV in;
		in.u = p.x / CARTESIAN_SCALAR;
		in.v = p.y / CARTESIAN_SCALAR;

		projUV out = pj_inv(in, m_proj);

		return out;
	}
};
