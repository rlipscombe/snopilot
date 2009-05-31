#pragma once

/* Because it's faster than a transition into whichever DLL CompareFileTime is in. */
inline int FastCompareFileTime(const FILETIME *lhs, const FILETIME *rhs)
{
	ASSERT(lhs && rhs);

	if (lhs->dwHighDateTime == rhs->dwHighDateTime)
	{
		if (lhs->dwLowDateTime == rhs->dwLowDateTime)
			return 0;
		else if (lhs->dwLowDateTime < rhs->dwLowDateTime)
			return -1;
		else
			return 1;
	}
	else if (lhs->dwHighDateTime < rhs->dwHighDateTime)
	{
		return -1;
	}
	else
		return 1;
}

inline bool operator <= (const FILETIME &lhs, const FILETIME &rhs)
{
	return (FastCompareFileTime(&lhs, &rhs) <= 0);
}

inline bool operator < (const FILETIME &lhs, const FILETIME &rhs)
{
	return (FastCompareFileTime(&lhs, &rhs) < 0);
}

inline bool operator > (const FILETIME &lhs, const FILETIME &rhs)
{
	return  (FastCompareFileTime(&lhs, &rhs) > 0);
}

inline bool operator >= (const FILETIME &lhs, const FILETIME &rhs)
{
	return  (FastCompareFileTime(&lhs, &rhs) >= 0);
}

inline void operator+= (FILETIME &lhs, const FILETIME &rhs)
{
	UINT64 llhs = ((UINT64)lhs.dwHighDateTime << 32) | lhs.dwLowDateTime;
	UINT64 rrhs = ((UINT64)rhs.dwHighDateTime << 32) | rhs.dwLowDateTime;

	llhs += rrhs;

	lhs.dwHighDateTime = (DWORD)((llhs & 0xffffffff00000000) >> 32);
	lhs.dwLowDateTime  = (DWORD) (llhs & 0x00000000ffffffff);
}

inline FILETIME operator+ (const FILETIME &lhs, const FILETIME &rhs)
{
	FILETIME result(lhs);
	result += rhs;
	return result;
}

inline ULARGE_INTEGER FileTimeToULargeInteger(const FILETIME &ft)
{
	ULARGE_INTEGER uli;

	uli.HighPart = ft.dwHighDateTime;
	uli.LowPart = ft.dwLowDateTime;

	return uli;
}

inline FILETIME ULargeIntegerToFileTime(const ULARGE_INTEGER &uli)
{
	FILETIME ft;

	ft.dwHighDateTime = uli.HighPart;
	ft.dwLowDateTime = uli.LowPart;

	return ft;
}

// See http://support.microsoft.com/default.aspx?scid=KB;en-us;q167296
inline FILETIME FileTimeFromUnixTime(time_t t)
{
    // Note that LONGLONG is a 64-bit value
    LONGLONG ll;

    ll = Int32x32To64(t, 10000000) + 116444736000000000;
    
	FILETIME ft;
	ft.dwLowDateTime = (DWORD)ll;
    ft.dwHighDateTime = (DWORD)(ll >> 32);

	return ft;
}

time_t GetUnixTimeFromSystemTime(SYSTEMTIME st);

// FILETIME is specified in 100ns intervals.  This means that there are 10 of them to 1us,
// and therefore 10,000,000 of them to 1 sec.
const unsigned int FILETIME_TICKS_PER_SECOND = 10000000;
const unsigned int FILETIME_TICKS_PER_MILLISECOND = 10000;

inline ULONGLONG FileTimeDifferenceInSeconds(const FILETIME &lhs, const FILETIME &rhs)
{
	ULARGE_INTEGER uli1 = FileTimeToULargeInteger(lhs);
	ULARGE_INTEGER uli2 = FileTimeToULargeInteger(rhs);

	ULONGLONG diff = uli1.QuadPart - uli2.QuadPart;
	return (int)(diff / FILETIME_TICKS_PER_SECOND);
}

inline FILETIME FileTimeFromSeconds(unsigned int seconds)
{
	ULARGE_INTEGER uli;
	uli.QuadPart = Int32x32To64(seconds, FILETIME_TICKS_PER_SECOND);

	return ULargeIntegerToFileTime(uli);
}
