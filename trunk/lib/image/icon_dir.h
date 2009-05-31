/* icon_dir.h
 *
 * Copyright (c) 2003 Roger Lipscombe, http://www.differentpla.net/~roger/
 */

#ifndef ICON_DIR_H
#define ICON_DIR_H 1

#include <pshpack1.h>
struct ICONDIRENTRY
{
    BYTE bWidth;
    BYTE bHeight;
    BYTE bColorCount;
    BYTE bReserved;	// must be zero
    WORD wPlanes;
    WORD wBitCount;
    DWORD dwBytesInRes;
};

struct ICONDIRENTRY_ICO
{
    ICONDIRENTRY hdr;
    DWORD dwImageOffset;    // offset, bytes, beginning of file -> image data
};

struct ICONDIRENTRY_EXE
{
    ICONDIRENTRY hdr;
    WORD wResourceIndex;
};

struct ICONDIR
{
    WORD idReserved;	// must be zero
    WORD idType;	// must be 1
    WORD idCount;
//  followed by:
//  ICONDIRENTRY idEntries[idCount];
};
#include <poppack.h>

#endif /* ICON_DIR_H */
