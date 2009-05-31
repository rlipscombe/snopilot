/* VersionInfo.h
 *
 * Copyright (C) 2003 Roger Lipscombe
 *     http://www.differentpla.net/~roger/
 */

bool VerifyDllVersionInfo(const char *dll, DWORD dwReqdVersion);

/** You can't use this definition of Wizard97 unless you're targetting IE 5.0 or later.
 * For some reason, Microsoft changed the value of PSH_WIZARD97 between IE4 and IE5.
 * Because I haven't thought of a reasonable work-around yet, Wizard97 is therefore
 * not supported unless you're targetting IE5 or newer
 */
#if _WIN32_IE >= 0x0500

/** Ironically, Windows 98 (stock) doesn't support Wizard97.  Windows 98SE does.
 * Actually, we need IE5, so we'll check the comctl32.dll version here...
 */
inline bool SupportsWizard97()
{
#define WIZARD97_VERSION_NEEDED MAKELONG(80,5)
    return VerifyDllVersionInfo("comctl32.dll", WIZARD97_VERSION_NEEDED);
}

#endif /* _WIN32_IE >= 0x0500 */

inline bool IsComCtl6()
{
#define COMCTL6_VERSION MAKELONG(0,6)
	return VerifyDllVersionInfo("comctl32.dll", COMCTL6_VERSION);
}

class CVersionInfo
{
	BYTE *m_buffer;
	WORD m_langId;
	WORD m_codePage;

public:
	CVersionInfo()
		: m_buffer(NULL), m_langId(0), m_codePage(0)
	{
	}

	~CVersionInfo()
	{
		free(m_buffer);
	}

	BOOL Create();
	BOOL Create(LPCTSTR lpszModule);

	CString GetString(const char *s);
};
