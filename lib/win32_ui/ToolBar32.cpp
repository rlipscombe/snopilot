#include "StdAfx.h"
#include "ToolBar32.h"
#include "image/bitmap_image.h"
#include "image/transform_dib.h"
#include "image/dib_transforms.h"

BOOL Load32BitColorToolBar(CToolBar *pToolBar, UINT nIDBitmap)
{
	// Loading 32-bit images into an image list.
	HBITMAP hBitmap = (HBITMAP)LoadImage(AfxFindResourceHandle(MAKEINTRESOURCE(nIDBitmap), MAKEINTRESOURCE(RT_BITMAP)), MAKEINTRESOURCE(nIDBitmap),
		IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_DEFAULTSIZE);
	if (!hBitmap)
		return FALSE;

	BITMAP bitmapObject;
	::GetObject(hBitmap, sizeof(BITMAP), &bitmapObject);

	ASSERT((bitmapObject.bmWidth % bitmapObject.bmHeight) == 0);	// Assume that the images are square.
	CSize imageSize(bitmapObject.bmHeight, bitmapObject.bmHeight);
	int imageCount = bitmapObject.bmWidth / bitmapObject.bmHeight;

	CBitmap normalBitmap;
	normalBitmap.Attach(hBitmap);

	// OK, the bitmap we've just loaded is a DIB section, we'll munge it, turn it into an image list and whack it into the toolbar.
	CImageList *normalImageList = NEW CImageList;
	normalImageList->Create(imageSize.cx, imageSize.cy, ILC_COLOR32, imageCount, 1);
	normalImageList->Add(&normalBitmap, (CBitmap *)NULL);

	// Right, so now munge it into a hot image list.
	// Get the BITMAPINFO structure for the bitmap.
	BITMAPINFO *bmi = (BITMAPINFO *)_alloca(sizeof(BITMAPINFOHEADER) + 256*sizeof(RGBQUAD));
	memset(&bmi->bmiHeader, 0, sizeof(BITMAPINFOHEADER));
	bmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);

	CWindowDC dc(NULL);
	int scanLineCount = GetDIBits(dc.GetSafeHdc(), hBitmap, 0, bitmapObject.bmHeight, NULL, bmi, DIB_RGB_COLORS);

	// Allocate a buffer for the bits.
	int imageBytes = dib::GetBitmapBytes(&bmi->bmiHeader);
	char *pSourceBits = (char *)malloc(imageBytes);

	// Get the bits.
	scanLineCount = GetDIBits(dc.GetSafeHdc(), hBitmap, 0, bitmapObject.bmHeight, pSourceBits, bmi, DIB_RGB_COLORS);

	// Munge the bitmap.
	CBitmap hotBitmap;
	hotBitmap.Attach(dib::CreateBitmapWithTransform(&dc, bmi, pSourceBits, dib::SaturationTransform()));

	CImageList *hotImageList = NEW CImageList;
	hotImageList->Create(imageSize.cx, imageSize.cy, ILC_COLOR32, imageCount, 1);
	hotImageList->Add(&hotBitmap, (CBitmap *)NULL);


	// And now munge it into a disabled image list -- we already have the bits lying around.
	CBitmap disabledBitmap;
	disabledBitmap.Attach(dib::CreateBitmapWithTransform(&dc, bmi, pSourceBits, dib::GrayscaleTransform()));

	CImageList *disabledImageList = NEW CImageList;
	disabledImageList->Create(imageSize.cx, imageSize.cy, ILC_COLOR32, imageCount, 1);
	disabledImageList->Add(&disabledBitmap, (CBitmap *)NULL);

	pToolBar->GetToolBarCtrl().SetImageList(normalImageList);
	pToolBar->GetToolBarCtrl().SetHotImageList(hotImageList);
	pToolBar->GetToolBarCtrl().SetDisabledImageList(disabledImageList);

	CSize buttonSize(imageSize.cx + 7, imageSize.cy + 6);
	pToolBar->SetSizes(buttonSize, imageSize);

	free(pSourceBits);

	return TRUE;
}
