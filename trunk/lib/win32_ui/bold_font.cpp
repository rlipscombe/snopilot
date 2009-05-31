#include "StdAfx.h"
#include "bold_font.h"

void GetMessageBoxFont(LOGFONT *lf)
{
	NONCLIENTMETRICS ncm;
	ncm.cbSize = sizeof(NONCLIENTMETRICS);
	VERIFY(SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0));
	*lf = ncm.lfMessageFont;
}
