#pragma once

class ApplicationProfile;

class NewFileSettings
{
	bool m_bCustomResort;

	CString m_resortFilePath;
	CString m_strCustomResortName;

	CString m_strComments;

public:
	NewFileSettings()
		: m_bCustomResort(true)
	{
	}

	void Load(const ApplicationProfile *pProfile);
	void Save(ApplicationProfile *pProfile);

public:
	void SetCustomResort(bool b) { m_bCustomResort = b; }
	bool IsCustomResort() const { return m_bCustomResort; }

	void SetResortPath(const CString &s) { m_resortFilePath = s; }
	CString GetResortPath() const { return m_resortFilePath; }

	void SetCustomResortName(const CString &s) { m_strCustomResortName = s; }
	CString GetCustomResortName() const { return m_strCustomResortName; }

	void SetComments(const CString &s) { m_strComments = s; }
	CString GetComments() const { return m_strComments; }
};
