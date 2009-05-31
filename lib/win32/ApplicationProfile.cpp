#include "StdAfx.h"
#include "ApplicationProfile.h"
#include "safe_snprintf.h"

// AfxApplicationProfile
CString AfxApplicationProfile::GetString(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszDefault) const
{
	return m_pApp->GetProfileString(lpszSection, lpszEntry, lpszDefault);
}

void AfxApplicationProfile::WriteString(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszValue)
{
	m_pApp->WriteProfileString(lpszSection, lpszEntry, lpszValue);
}

int AfxApplicationProfile::GetInteger(LPCTSTR lpszSection, LPCTSTR lpszEntry, int nDefault) const
{
	return m_pApp->GetProfileInt(lpszSection, lpszEntry, nDefault);
}

void AfxApplicationProfile::WriteInteger(LPCTSTR lpszSection, LPCTSTR lpszEntry, int nValue)
{
	m_pApp->WriteProfileInt(lpszSection, lpszEntry, nValue);
}

void AfxApplicationProfile::EnumerateSections(SectionCallback *pCallback)
{
}

// PrivateApplicationProfile
CString PrivateApplicationProfile::GetString(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszDefault) const
{
	CString str;
	DWORD dw = ::GetPrivateProfileString(lpszSection, lpszEntry, lpszDefault, str.GetBuffer(MAX_PATH), MAX_PATH, m_strFileName);
	str.ReleaseBuffer();

	return str;
}

void PrivateApplicationProfile::WriteString(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszValue)
{
	::WritePrivateProfileString(lpszSection, lpszEntry, lpszValue, m_strFileName);
}

int PrivateApplicationProfile::GetInteger(LPCTSTR lpszSection, LPCTSTR lpszEntry, int nDefault) const
{
	return ::GetPrivateProfileInt(lpszSection, lpszEntry, nDefault, m_strFileName);
}

void PrivateApplicationProfile::WriteInteger(LPCTSTR lpszSection, LPCTSTR lpszEntry, int nValue)
{
	CString str;
	str.Format(_T("%d"), nValue);
	::WritePrivateProfileString(lpszSection, lpszEntry, str, m_strFileName);
}

void PrivateApplicationProfile::EnumerateSections(SectionCallback *pCallback)
{
	const size_t cchSectionNames = 10240;
	TCHAR *pSectionNames = new TCHAR[cchSectionNames];
	::GetPrivateProfileSectionNames(pSectionNames, cchSectionNames, m_strFileName);

	for (const TCHAR *p = pSectionNames; p && *p; p += _tcslen(p)+1)
	{
		pCallback->OnSection(this, p);
	}

	delete [] pSectionNames;
}

CString MemoryApplicationProfile::GetString(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszDefault) const
{
	sections_t::const_iterator i = m_sections.find(lpszSection);
	if (i != m_sections.end())
	{
		const section_t &section = i->second;
		section_t::const_iterator j = section.find(lpszEntry);
		if (j != section.end())
			return j->second.c_str();
	}

	return lpszDefault;
}

void MemoryApplicationProfile::WriteString(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszValue)
{
	m_sections[lpszSection][lpszEntry] = lpszValue;
}

int MemoryApplicationProfile::GetInteger(LPCTSTR lpszSection, LPCTSTR lpszEntry, int nDefault) const
{
	CString strDefault;
	strDefault.Format(_T("%d"), nDefault);

	CString value = GetString(lpszSection, lpszEntry, strDefault);

	_TCHAR *p;
	long n = _tcstol(value, &p, 10);
	ASSERT(!*p);	// There was a string stored, but we're asking for an integer?

	return (int)n;
}

void MemoryApplicationProfile::WriteInteger(LPCTSTR lpszSection, LPCTSTR lpszEntry, int nValue)
{
	_TCHAR temp[MAX_PATH];
	safe_sntprintf(temp, MAX_PATH, _T("%d"), nValue);

	WriteString(lpszSection, lpszEntry, temp);
}

void MemoryApplicationProfile::EnumerateSections(SectionCallback *pCallback)
{
	for (sections_t::const_iterator i = m_sections.begin(); i != m_sections.end(); ++i)
	{
		pCallback->OnSection(this, i->first.c_str());
	}
}

#if defined(_DEBUG)
#include "test/test_assert.h"
#include <algorithm>

void test_MemoryApplicationProfile()
{
	MemoryApplicationProfile profile;

	// Test behaviour when it's empty.
	assertStringsEqual(_T(""), profile.GetString(_T("Section"), _T("Missing"), _T("")));
	assertStringsEqual(_T("Default"), profile.GetString(_T("Section"), _T("Missing"), _T("Default")));

	// Check that we can set/get one string, and that it's still OK for missing values.
	profile.WriteString("Section", "Entry", "Value");
	assertStringsEqual(_T("Value"), profile.GetString(_T("Section"), _T("Entry"), _T("Default")));
	assertStringsEqual(_T("Default"), profile.GetString(_T("Section"), _T("Missing"), _T("Default")));

	// Check that we can set/get a different string, and that our earlier string is still in there.
	profile.WriteString("Section2", "Entry2", "Value2");
	assertStringsEqual(_T("Value2"), profile.GetString(_T("Section2"), _T("Entry2"), _T("Default")));
	assertStringsEqual(_T("Value"), profile.GetString(_T("Section"), _T("Entry"), _T("Default")));

	// Test it with some integers: empty.
	assertIntegersEqual(0, profile.GetInteger("Section3", "Missing", 0));
	assertIntegersEqual(42, profile.GetInteger("Section3", "Missing", 42));

	// Integers: get/set/missing.
	profile.WriteInteger("Section3", "Entry3", 69);
	assertIntegersEqual(69, profile.GetInteger("Section3", "Entry3", 42));
	assertIntegersEqual(12, profile.GetInteger("Section3", "Missing", 12));

	profile.WriteInteger("Section4", "Entry4", 123);
	assertIntegersEqual(69, profile.GetInteger("Section3", "Entry3", 42));
	assertIntegersEqual(123, profile.GetInteger("Section4", "Entry4", 0));

	profile.WriteString("Section 5", "Entry 5", "");
	assertStringsEqual("", profile.GetString("Section 5", "Entry 5", "Default"));

	std::vector< std::string > v;
	ApplicationProfileSectionCollector sectionCollector(&v);
	profile.EnumerateSections(&sectionCollector);

	assertTrue(std::find(v.begin(), v.end(), "Section") != v.end());
	assertTrue(std::find(v.begin(), v.end(), "Section2") != v.end());
	assertTrue(std::find(v.begin(), v.end(), "Section3") != v.end());
	assertTrue(std::find(v.begin(), v.end(), "Section4") != v.end());
	assertTrue(std::find(v.begin(), v.end(), "Section 5") != v.end());
}
#endif
