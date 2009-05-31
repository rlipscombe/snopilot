/* dump_ico.cpp
 *
 * Copyright (c) 2003 Roger Lipscombe, http://www.differentpla.net/~roger/
 */

#include "config.h"
#include "trace.h"
#include "dump_ico.h"

#define TRACE_ICO 0

namespace ico {
    void DumpIconDir(const ICONDIR *pIconDir)
    {
	TRACEC(TRACE_ICO, "typedef struct ICONDIR {\n");
	TRACEC(TRACE_ICO, "    WORD idReserved = %d;\n", pIconDir->idReserved);
	TRACEC(TRACE_ICO, "    WORD idType = %d;\n", pIconDir->idType);
	TRACEC(TRACE_ICO, "    WORD idCount = %d;\n", pIconDir->idCount);
	
	const ICONDIRENTRY_EXE *pIconDirEntry = (const ICONDIRENTRY_EXE *)(pIconDir + 1);
	for (int i = 0; i < pIconDir->idCount; ++i)
	{
	    TRACEC(TRACE_ICO, "    ICONDIRENTRY idEntries[%d] = {\n", i);
	    
	    TRACEC(TRACE_ICO, "        BYTE bWidth = %d;\n", pIconDirEntry->hdr.bWidth);
	    TRACEC(TRACE_ICO, "        BYTE bHeight = %d;\n", pIconDirEntry->hdr.bHeight);
	    TRACEC(TRACE_ICO, "        BYTE bColorCount = %d;\n", pIconDirEntry->hdr.bColorCount);
	    TRACEC(TRACE_ICO, "        BYTE bReserved = %d;\n", pIconDirEntry->hdr.bReserved);
	    TRACEC(TRACE_ICO, "        WORD wPlanes = %d;\n", pIconDirEntry->hdr.wPlanes);
	    TRACEC(TRACE_ICO, "        WORD wBitCount = %d;\n", pIconDirEntry->hdr.wBitCount);
	    TRACEC(TRACE_ICO, "        DWORD dwBytesInRes = %d;\n", pIconDirEntry->hdr.dwBytesInRes);
	    TRACEC(TRACE_ICO, "        WORD wResourceIndex = %d;\n", pIconDirEntry->wResourceIndex);
	    ++pIconDirEntry;
	}
	TRACEC(TRACE_ICO, "    };\n");
	TRACEC(TRACE_ICO, "} ICONDIR;\n");
    }
};  // namespace ico

