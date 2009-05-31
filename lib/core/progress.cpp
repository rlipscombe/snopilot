#include "config.h"
#include "trace.h"
#include "progress.h"
#include <stdio.h>

STATUS TextProgress::OnProgress(int num, int denom)
{
	if (denom != 0)
	{
		int pct = num * 100 / denom;
		printf("\r%d / %d = %-3d%%    ", num, denom, pct);
	}

	return S_OK;
}

STATUS TextProgress::OnComplete()
{
	printf("\n");
	return S_OK;
}
