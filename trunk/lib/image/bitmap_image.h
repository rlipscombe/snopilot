/* bitmap_image.h
 *
 * Copyright (c) 2003 Roger Lipscombe, http://www.differentpla.net/~roger/
 * Copyright (c) 2004 snopilot, http://www.snopilot.com/
 */

namespace dib {

	inline bool IsValidBitmapFileHeader(const BITMAPFILEHEADER *bmfh)
	{
		// It should start with "BM" and (for paranoia) have two zero values in reserved slots.
		return (bmfh->bfType == MAKEWORD('B', 'M') && bmfh->bfReserved1 == 0 && bmfh->bfReserved2 == 0);
	}

	inline bool HasColourTable(const BITMAPINFOHEADER *bmih)
	{
		// 1bpp, 2bpp, 4bpp and 8bpp bitmaps have colour tables.
		return (bmih->biBitCount == 1) || (bmih->biBitCount == 2) ||
			(bmih->biBitCount == 4) || (bmih->biBitCount == 8);
	}

	inline int GetColourTableSize(const BITMAPINFOHEADER *bmih)
	{
		int colourCount = 0;
		if (bmih->biClrUsed == 0)
		{
			colourCount = 1;
			for (int i = 0; i < bmih->biBitCount; ++i)
				colourCount *= 2;
		}
		else
		{
			colourCount = bmih->biClrUsed;
		}

		return colourCount;
	}

	inline int GetBytesPerPixel(int depth)
	{
		return (depth==32 ? 4 : 3);
	}

	inline int GetBytesPerRow(int width, int depth)
	{
		int bytesPerPixel = GetBytesPerPixel(depth);
		int bytesPerRow = ((width * bytesPerPixel + 3) & ~3);

		return bytesPerRow;
	}

	inline int GetBitmapBytes(int width, int height, int depth)
	{
		return height * GetBytesPerRow(width, depth);
	}

	inline int GetBitmapBytes(const BITMAPINFOHEADER *bmih)
	{
		return GetBitmapBytes(bmih->biWidth, bmih->biHeight, bmih->biBitCount);
	}

	inline RGBTRIPLE *GetPixelPtr(char *pixelData, int width, int height, int depth, int x, int y)
	{
		if (x < 0 || x >= width || y < 0 || y >= height)
			return NULL;

		int bytesPerRow = GetBytesPerRow(width, depth);
		int bytesPerPixel = GetBytesPerPixel(depth);

		int rowOffset = y * bytesPerRow;	// Gets us to the correct row.
		int colOffset = x * bytesPerPixel;	// Gets us to the correct column.

		int pixelOffset = rowOffset + colOffset;

		return (RGBTRIPLE *)(pixelData + pixelOffset);
	}

}; // namespace dib
