/* dump_dib.cpp
 *
 * Copyright (c) 2003 Roger Lipscombe, http://www.differentpla.net/~roger/
 * Copyright (c) 2004 snopilot, http://www.snopilot.com/
 */

#include "config.h"
#include "trace.h"
#include "dump_dib.h"
#include "bitmap_image.h"

#define TRACE_DIB 0
#define TRACE_COLOURTABLE 0
#define TRACE_PIXELDATA 0

namespace dib {

	void DumpBitmapFile(const BYTE *pBuffer, unsigned cbBuffer)
	{
		// Go spelunking in the file:  It starts with a BITMAPFILEHEADER
		const BITMAPFILEHEADER *bmfh = (const BITMAPFILEHEADER *)pBuffer;
		if (IsValidBitmapFileHeader(bmfh))
		{
			// Then it's a valid bitmap.  Dump out some other useful information.
			TRACEC(TRACE_DIB, "It's a valid bitmap.\n");

			TRACEC(TRACE_DIB, "bfType = %c%c\n",	LOBYTE(bmfh->bfType), HIBYTE(bmfh->bfType));
			TRACEC(TRACE_DIB, "bfSize = %d\n",	bmfh->bfSize);
			TRACEC(TRACE_DIB, "bfReserved1 = %d\n", bmfh->bfReserved1);
			TRACEC(TRACE_DIB, "bfReserved2 = %d\n", bmfh->bfReserved2);
			TRACEC(TRACE_DIB, "bfOffBits = %d\n",   bmfh->bfOffBits);
		}

		// Immediately following that is supposed to be a BITMAPINFO structure:
		const BITMAPINFO *bmi = (const BITMAPINFO *)(pBuffer + sizeof(BITMAPFILEHEADER));

		// This consists of a BITMAPINFOHEADER, followed by a colour table (in RGBQUADs).
		// We'll dump the BITMAPINFOHEADER as well.
		DumpBitmapInfoHeader(&bmi->bmiHeader);
	}

	void DumpBitmapInfoHeader(const BITMAPINFOHEADER *pBitmapInfoHeader)
	{
		if (pBitmapInfoHeader->biSize == sizeof(BITMAPV5HEADER))
			TRACEC(TRACE_DIB, "It's a BITMAPV5HEADER.\n");
		else if (pBitmapInfoHeader->biSize == sizeof(BITMAPV4HEADER))
			TRACEC(TRACE_DIB, "It's a BITMAPV4HEADER.\n");
		else if (pBitmapInfoHeader->biSize == sizeof(BITMAPINFOHEADER))
			TRACEC(TRACE_DIB, "It's a BITMAPINFOHEADER.\n");
		else
			TRACEC(TRACE_DIB, "Not sure what it is, treating it as a BITMAPINFOHEADER.\n");

		TRACEC(TRACE_DIB, "typedef struct tagBITMAPINFOHEADER {\n");
		TRACEC(TRACE_DIB, "    DWORD  biSize = %d;\n", pBitmapInfoHeader->biSize);
		TRACEC(TRACE_DIB, "    LONG   biWidth = %d;\n", pBitmapInfoHeader->biWidth);
		TRACEC(TRACE_DIB, "    LONG   biHeight = %d;\n", pBitmapInfoHeader->biHeight);
		TRACEC(TRACE_DIB, "    WORD   biPlanes = %d;\n", pBitmapInfoHeader->biPlanes);
		TRACEC(TRACE_DIB, "    WORD   biBitCount = %d;\n", pBitmapInfoHeader->biBitCount);
		TRACEC(TRACE_DIB, "    DWORD  biCompression = %d (%s);\n", pBitmapInfoHeader->biCompression,
			pBitmapInfoHeader->biCompression == BI_RGB ? "BI_RGB" :
		pBitmapInfoHeader->biCompression == BI_RLE8 ? "BI_RLE8" :
		pBitmapInfoHeader->biCompression == BI_RLE4 ? "BI_RLE4" :
		pBitmapInfoHeader->biCompression == BI_BITFIELDS ? "BI_BITFIELDS" :
		"unknown");
		TRACEC(TRACE_DIB, "    DWORD  biSizeImage = %d;\n", pBitmapInfoHeader->biSizeImage);
		TRACEC(TRACE_DIB, "    LONG   biXPelsPerMeter = %d;\n", pBitmapInfoHeader->biXPelsPerMeter);
		TRACEC(TRACE_DIB, "    LONG   biYPelsPerMeter = %d;\n", pBitmapInfoHeader->biYPelsPerMeter);
		TRACEC(TRACE_DIB, "    DWORD  biClrUsed = %d;\n", pBitmapInfoHeader->biClrUsed);
		TRACEC(TRACE_DIB, "    DWORD  biClrImportant = %d;\n", pBitmapInfoHeader->biClrImportant);
		TRACEC(TRACE_DIB, "} BITMAPINFOHEADER;\n");
	}

	void DumpColourTable(const BITMAPINFO *bmi)
	{
		int colourCount = GetColourTableSize(&bmi->bmiHeader);
		TRACEC(TRACE_COLOURTABLE, "Colour table contains %d entries.\n", colourCount);

		DumpColourTable(bmi->bmiColors, colourCount);
	}

	void DumpColourTable(const RGBQUAD *colourTable, int colourCount)
	{
		for (int i = 0; i < colourCount; ++i)
		{
			/** @todo Format these in HTML style? */
			TRACEC(TRACE_COLOURTABLE, "bmiColors[%d] = BGRA(%d,%d,%d,%d)\n",
				i, colourTable[i].rgbBlue,
				colourTable[i].rgbGreen,
				colourTable[i].rgbRed,
				colourTable[i].rgbReserved);
		}
	}

	void DumpPixelData32(const BITMAPINFOHEADER *bmih)
	{
		ASSERT(bmih->biBitCount == 32);

		// The bitmap begins with a BITMAPINFOHEADER, then it's followed by the pixel
		// data.  Let's take a look at that:
		const RGBQUAD *pPixels = (const RGBQUAD *) ((const BYTE *)bmih) + bmih->biSize;
		const RGBQUAD *rgbq = pPixels;
		for (int y = 0; y < bmih->biHeight; ++y)
		{
			for (int x = 0; x < bmih->biWidth; ++x)
			{
				TRACEC(TRACE_PIXELDATA, "BGRA(%d,%d,%d,%d)\n", rgbq->rgbBlue, rgbq->rgbGreen, rgbq->rgbRed, rgbq->rgbReserved);
				++rgbq;
			}
		}
    }
    
}; // namespace dib
