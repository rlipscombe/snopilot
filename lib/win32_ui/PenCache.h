#pragma once
#include <map>

/** PenCache
 *
 * Because creating/selecting pen objects into DCs is actually quite expensive, we
 * can cache the creation of the pen objects using a PenCache.
 *
 * It appears to work: it shaves 180ms off of the drawing time for Whistler/Blackcomb.
 */
class PenCache
{
	typedef std::map< DWORD, CPen * > cache_t;
	cache_t m_cache;

public:
	PenCache()
	{
		// Put the most likely candidate into the cache.
		GetPen(PS_SOLID, 1, RGB(0,0,0));
	}

	~PenCache()
	{
		for (cache_t::iterator i = m_cache.begin(); i != m_cache.end(); ++i)
		{
			delete i->second;
		}

		m_cache.clear();
	}

public:
	CPen *GetPen(int penStyle, int penWidth, COLORREF penColor)
	{
		DWORD penHash = GeneratePenHash(penStyle, penWidth, penColor);

		CPen *pen = m_cache[penHash];
		if (pen)
			return pen;

		pen = NEW CPen(penStyle, penWidth, penColor);
		if (pen)
			m_cache[penHash] = pen;

		return pen;
	}

private:
	DWORD GeneratePenHash(int penStyle, int penWidth, COLORREF penColor)
	{
		// COLORREF is 0x00bbggrr, so we can use the top 8 bits to hash the other information.
		ASSERT(penStyle >= 0 && penStyle < 16);
		ASSERT(penWidth >= 0 && penWidth < 16);
		return (penStyle << 28) | (penWidth << 24) | penColor;
	}
};

