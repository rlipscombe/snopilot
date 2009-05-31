#include "StdAfx.h"
#include "NewFileSettings.h"
#include "win32/ApplicationProfile.h"

const TCHAR newFileSection[] = _T("New File Defaults");

void NewFileSettings::Load(const ApplicationProfile *pProfile)
{
	m_bCustomResort = pProfile->GetBoolean(newFileSection, "Custom Resort", true);

	m_resortFilePath = pProfile->GetString(newFileSection, "Resort File", "");
	m_strCustomResortName = pProfile->GetString(newFileSection, "Custom Resort Name", "");

	m_strComments = pProfile->GetString(newFileSection, "Comments", "");
}

void NewFileSettings::Save(ApplicationProfile *pProfile)
{
	pProfile->WriteBoolean(newFileSection, "Custom Resort", m_bCustomResort);

	pProfile->WriteString(newFileSection, "Resort File", m_resortFilePath);
	pProfile->WriteString(newFileSection, "Custom Resort Name", m_strCustomResortName);

	pProfile->WriteString(newFileSection, "Comments", m_strComments);
}
