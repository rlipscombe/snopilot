#include "config.h"
#include "trace.h"
#include "image/bitmap_image.h"

void SaveDib(HDC hDC, LPCTSTR lpszFileName, BOOL bOverwriteExisting)
{
	HBITMAP hBitmap = (HBITMAP)GetCurrentObject(hDC, OBJ_BITMAP);

	// The .BMP file format is as follows:
	// BITMAPFILEHEADER / BITMAPINFOHEADER / color table / pixel data

	// We need the pixel data and the BITMAPINFOHEADER.  We can get both by using GetDIBits:
	BITMAP bitmapObject;
	GetObject(hBitmap, sizeof(BITMAP), &bitmapObject);

	BITMAPINFO *bmi = (BITMAPINFO *)_alloca(sizeof(BITMAPINFOHEADER) + 256*sizeof(RGBQUAD));
	memset(&bmi->bmiHeader, 0, sizeof(BITMAPINFOHEADER));
	bmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	int scanLineCount = GetDIBits(hDC, hBitmap, 0, bitmapObject.bmHeight, NULL, bmi, DIB_RGB_COLORS);

	// This is important: GetDIBits will, by default, set this to BI_BITFIELDS.
	bmi->bmiHeader.biCompression = BI_RGB;

	int imageBytes = dib::GetBitmapBytes(&bmi->bmiHeader);
	char *pBits = (char *)malloc(imageBytes);

	scanLineCount = GetDIBits(hDC, hBitmap, 0, bitmapObject.bmHeight, pBits, bmi, DIB_RGB_COLORS);

	// OK, so we've got the bits, and the BITMAPINFOHEADER.  Now we can put them in a file.
	HANDLE hFile = CreateFile(lpszFileName, GENERIC_WRITE, 0, NULL, bOverwriteExisting ? CREATE_ALWAYS : CREATE_NEW, 0, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		// We don't deal with writing anything else at the moment.
		ASSERT(bmi->bmiHeader.biBitCount == 32);

		// .BMP file begins with a BITMAPFILEHEADER, so we'll write that.
		BITMAPFILEHEADER bmfh;
		bmfh.bfType = MAKEWORD('B','M');
		bmfh.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + (bmi->bmiHeader.biClrUsed * sizeof(RGBQUAD)) + bmi->bmiHeader.biSizeImage;
		bmfh.bfReserved1 = 0;
		bmfh.bfReserved2 = 0;
		bmfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + (bmi->bmiHeader.biClrUsed * sizeof(RGBQUAD));

		DWORD bytesWritten;
		WriteFile(hFile, &bmfh, sizeof(BITMAPFILEHEADER), &bytesWritten, NULL);

		// Then it's followed by the BITMAPINFOHEADER structure
		WriteFile(hFile, &bmi->bmiHeader, sizeof(BITMAPINFOHEADER), &bytesWritten, NULL);
		
		// Then the colour table.
		// ...which we don't support yet.

		// Then the pixel data.
		WriteFile(hFile, pBits, imageBytes, &bytesWritten, NULL);

		CloseHandle(hFile);
	}

	free(pBits);
}

