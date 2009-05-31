#pragma once

inline float DegreesFromSemicircles(long semicircles)
{
	return (float)((double)semicircles * 180) / 2147483647;
}

inline long SemicirclesFromDegrees(float degrees)
{
	return (long)((double)degrees * ((double)2147483647 / 180));
}
