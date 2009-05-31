/* transform_dib.h
 */

#ifndef TRANSFORM_DIB_H
#define TRANSFORM_DIB_H 1

namespace dib
{
	template <class Transform>
		HBITMAP CreateBitmapWithTransform(CDC *pDC, const BITMAPINFO *bmi, const void *pSourceBits, const Transform & trans)
	{
		void *pDib;
		HBITMAP hbm = CreateDIBSection(pDC->GetSafeHdc(), bmi, DIB_RGB_COLORS, &pDib, NULL, 0);

		// That's given us a blank bitmap.  Now transform it:
		const RGBQUAD *pSource = (const RGBQUAD *)pSourceBits;
		RGBQUAD *pDest = (RGBQUAD *)pDib;

		for (int y = 0; y < bmi->bmiHeader.biHeight; ++y)
		{
			for (int x = 0; x < bmi->bmiHeader.biWidth; ++x)
			{
				int offset = x + (bmi->bmiHeader.biWidth * y);

				RGBQUAD src = pSource[offset];
				RGBQUAD dst = trans(src);
				pDest[offset] = dst;
			}
		}

		return hbm;
	}

	template <class Transform>
		HBITMAP CreateBitmapWithTransform(CDC *pDC, const BITMAPINFO *bmi,
		const BITMAPFILEHEADER *bmfh, const BITMAPINFOHEADER *bmih,
		const Transform & trans)
	{
		return CreateBitmapWithTransform(pDC, bmi, ((const BYTE *)bmfh + bmfh->bfOffBits), trans);
	}
}; // namespace dib

#endif /* TRANSFORM_DIB_H */
