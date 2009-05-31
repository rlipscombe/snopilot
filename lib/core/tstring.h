/* roger_tchar.h
 *
 * Copyright (C) 2003 Roger Lipscombe, http://www.differentpla.net/~roger/
 */

#ifndef ROGER_TCHAR_H
#define ROGER_TCHAR_H 1

#include <string>

#if defined(_MSC_VER)
#include "tchar.h"
#include <xstring>

#if defined(UNICODE)
    typedef std::wstring tstring;
#else
    typedef std::string tstring;
#endif

#else
typedef char TCHAR;

#define _T(x) (x)

typedef std::string tstring;

#define _topen open
#define _tunlink unlink
#define _tstat stat
#define _tfopen fopen
#endif

#endif /* ROGER_TCHAR_H */
