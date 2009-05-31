#pragma once
#include <map>
#include <vector>
#include "tstring.h"

/** Abstract base class for storing application preferences.
 */
class ApplicationProfile
{
public:
	virtual CString GetString(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszDefault) const = 0;
	virtual void WriteString(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszValue) = 0;

	virtual int GetInteger(LPCTSTR lpszSection, LPCTSTR lpszEntry, int nDefault) const = 0;
	virtual void WriteInteger(LPCTSTR lpszSection, LPCTSTR lpszEntry, int nValue) = 0;

	inline bool GetBoolean(LPCTSTR lpszSection, LPCTSTR lpszEntry, bool bDefault) const
	{
		return GetInteger(lpszSection, lpszEntry, bDefault) ? true : false;
	}

	inline void WriteBoolean(LPCTSTR lpszSection, LPCTSTR lpszEntry, bool bValue)
	{
		return WriteInteger(lpszSection, lpszEntry, bValue ? 1 : 0);
	}

	class SectionCallback
	{
	public:
		virtual void OnSection(ApplicationProfile *pProfile, LPCTSTR lpszSection) = 0;
	};

	virtual void EnumerateSections(SectionCallback *pCallback) = 0;
};

/** Implementation of ApplicationProfile interface that uses CWinApp::WriteProfileString, etc.
 */
class AfxApplicationProfile : public ApplicationProfile
{
	CWinApp *m_pApp;

public:
	AfxApplicationProfile(CWinApp *pApp)
		: m_pApp(pApp)
	{
	}

	virtual CString GetString(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszDefault) const;
	virtual void WriteString(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszValue);

	virtual int GetInteger(LPCTSTR lpszSection, LPCTSTR lpszEntry, int nDefault) const;
	virtual void WriteInteger(LPCTSTR lpszSection, LPCTSTR lpszEntry, int nValue);

	virtual void EnumerateSections(SectionCallback *pCallback);
};

/** Implementation of ApplicationProfile interface that uses ::WritePrivateProfileString, etc.
 */
class PrivateApplicationProfile : public ApplicationProfile
{
	CString m_strFileName;

public:
	PrivateApplicationProfile(const TCHAR *pszFileName)
		: m_strFileName(pszFileName)
	{
	}

	virtual CString GetString(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszDefault) const;
	virtual void WriteString(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszValue);

	virtual int GetInteger(LPCTSTR lpszSection, LPCTSTR lpszEntry, int nDefault) const;
	virtual void WriteInteger(LPCTSTR lpszSection, LPCTSTR lpszEntry, int nValue);

	virtual void EnumerateSections(SectionCallback *pCallback);
};

/** Implementation of ApplicationProfile that keeps the sections, keys and values in memory.
 */
class MemoryApplicationProfile : public ApplicationProfile
{
	// An ApplicationProfile object comprises a bunch of sections, each labelled with a string,
	// containing entry/value pairs, both strings.
	typedef std::map< tstring, tstring > section_t;
	typedef std::map< tstring, section_t > sections_t;

	sections_t m_sections;

// ApplicationProfile
public:
	virtual CString GetString(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszDefault) const;
	virtual void WriteString(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszValue);

	virtual int GetInteger(LPCTSTR lpszSection, LPCTSTR lpszEntry, int nDefault) const;
	virtual void WriteInteger(LPCTSTR lpszSection, LPCTSTR lpszEntry, int nValue);

	virtual void EnumerateSections(SectionCallback *pCallback);
};

/** Helper class: in case you'd prefer your profile sections in a vector<>
 */
class ApplicationProfileSectionCollector : public ApplicationProfile::SectionCallback
{
	std::vector< tstring > *m_vec;

public:
	ApplicationProfileSectionCollector(std::vector< tstring > *v)
		: m_vec(v)
	{
	}

	virtual void OnSection(ApplicationProfile *pProfile, LPCTSTR lpszSection)
	{
		m_vec->push_back(lpszSection);
	}
};
