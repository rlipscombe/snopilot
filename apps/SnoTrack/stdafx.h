// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#endif

#define _USE_MATH_DEFINES

#ifndef WINVER
#define WINVER 0x0501
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif						

#ifndef _WIN32_WINDOWS
#define _WIN32_WINDOWS 0x0501
#endif

#ifndef _WIN32_IE
#define _WIN32_IE 0x0600
#endif

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit

// turns off MFC's hiding of some common and often safely ignored warning messages
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions

#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxdlgs.h>

#include <afxpriv.h>		// for WM_SIZEPARENT etc.

#include <Shlwapi.h>

#include <math.h>
#include <float.h>

inline double DegreesToRadians(double degrees)
{
	return degrees * M_PI / 180;
}

inline double RadiansToDegrees(double radians)
{
	return 180 * radians / M_PI;
}

// 10 degrees and 5 degrees, respectively.
#define M_PI_18       0.17453292519943295769236907684886
#define M_PI_36       0.087266462599716478846184538424431

#include <proj-4.4.7/src/projects.h>

#include "config.h"
#include "trace.h"

// Set this to 1 if you want your MDI children initially maximized.
// Set it to zero for default behaviour.
#define MAXIMIZE_MDI_CHILDREN 1