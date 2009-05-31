/* VersionInfo.cpp
 *
 * Copyright (C) 2003 Roger Lipscombe
 *     http://www.differentpla.net/~roger/
 */

#include "StdAfx.h"
#include "VersionInfo.h"
#include <Shlwapi.h>
#pragma comment(lib, "version.lib")
#include "safe_snprintf.h"

/** Check that the DLL has at least the version required.  It assumes that the DLL
 * is already loaded.
 */
bool VerifyDllVersionInfo(const char *dll, DWORD dwReqdVersion)
{
	HINSTANCE hInst = ::GetModuleHandleA(dll);
	if (!hInst)
		return false;

	DLLGETVERSIONPROC pfn = (DLLGETVERSIONPROC)GetProcAddress(hInst, "DllGetVersion");
	if (pfn == NULL)
		return false;

	DLLVERSIONINFO dvi;
	memset(&dvi, 0, sizeof(dvi));
	dvi.cbSize = sizeof(dvi);
	HRESULT hr = pfn(&dvi);
	if (FAILED(hr))
		return false;

	if (!(dvi.dwMajorVersion <= 0xFFFF) || !(dvi.dwMinorVersion <= 0xFFFF))
		return false;

	DWORD dwVersion = MAKELONG(dvi.dwMinorVersion, dvi.dwMajorVersion);
	return (dwVersion >= dwReqdVersion);
}

BOOL CVersionInfo::Create()
{
	TCHAR moduleFileName[_MAX_PATH];
	GetModuleFileName(NULL, moduleFileName, COUNTOF(moduleFileName));

	return Create(moduleFileName);
}

BOOL CVersionInfo::Create(LPCTSTR lpszModule)
{
	DWORD dwHandle;	// Not actually used, but GetFileVersionInfoSize needs to set it to zero anyway.
	DWORD dwLen = GetFileVersionInfoSize(lpszModule, &dwHandle);
	if (dwLen == 0)
		return FALSE;

	m_buffer = (BYTE *)malloc(dwLen);
	if (!GetFileVersionInfo(lpszModule, dwHandle, dwLen, m_buffer))
	{
		free(m_buffer);
		m_buffer = NULL;
		return FALSE;
	}

#if 0
	// Get the VS_FIXEDFILEINFO information.  We don't actually do anything with it, but it's a useful example.
	VS_FIXEDFILEINFO *pFixed;
	UINT cbFixed;
	VERIFY(VerQueryValue(buffer, _T("\\"), (void **)&pFixed, &cbFixed));
	TRACE_HEX("VS_FIXEDFILEINFO", pFixed, pFixed + 1);
#endif

	// Now find out which translations are in the block.
	WORD *pTranslation;
	UINT cbTranslation;
	VERIFY(VerQueryValue(m_buffer, _T("\\VarFileInfo\\Translation"), (void **)&pTranslation, &cbTranslation));

	m_langId = pTranslation[0];
	m_codePage = pTranslation[1];

	return TRUE;
}

CString CVersionInfo::GetString(const char *s)
{
	TCHAR subBlockName[_MAX_PATH];
	_sntprintf(subBlockName, _MAX_PATH, _T("\\StringFileInfo\\%04x%04x\\%s"), m_langId, m_codePage, s);

	char *p;
	UINT cb;
	VERIFY(VerQueryValue(m_buffer, subBlockName, (void **)&p, &cb));

	return CString(p);
}

#if defined(_DEBUG)
#include "test/test_assert.h"

void test_CVersionInfo()
{
	CVersionInfo ver;
	assertFalse(ver.Create(_T("not_there")));

	assertTrue(ver.Create());
	assertStringsEqual(_T("snopilot"), ver.GetString("CompanyName"));
	assertStringsEqual(_T("snotrack"), ver.GetString("ProductName"));
	assertStringsEqual(_T("Copyright (c) 2003-2004 snopilot.  All rights reserved."), ver.GetString("LegalCopyright"));
}
#endif
