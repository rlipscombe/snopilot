#include "StdAfx.h"
#include "PostFileSettings.h"
#include "win32/ApplicationProfile.h"

void PostFileSettings::Load(ApplicationProfile *pProfile)
{
	m_strLocalFilename = pProfile->GetString(_T("Settings"), _T("Local File"), _T(""));

	m_bAgree = FALSE;//pProfile->GetInteger("Settings", "Agree", FALSE);

	m_bAnonymous = pProfile->GetInteger(_T("Contact Details"), _T("Anonymous"), FALSE);
	m_strEmailAddress = pProfile->GetString(_T("Contact Details"), _T("Email Address"), _T(""));
	m_strForumUserName = pProfile->GetString(_T("Contact Details"), _T("Forum Username"), _T(""));
}

void PostFileSettings::Save(ApplicationProfile *pProfile)
{
	pProfile->WriteString(_T("Settings"), _T("Local File"), m_strLocalFilename);

	//pProfile->WriteInteger("Settings", "Agree", m_bAgree);

	pProfile->WriteInteger(_T("Contact Details"), _T("Anonymous"), m_bAnonymous);
	pProfile->WriteString(_T("Contact Details"), _T("Email Address"), m_strEmailAddress);
	pProfile->WriteString(_T("Contact Details"), _T("Forum Username"), m_strForumUserName);
}

void PostFileResults::Load(ApplicationProfile *pProfile)
{
	// Assume we're on 56Kbps dialup by default.
	m_dwBytesPerSecond = pProfile->GetInteger(_T("Settings"), _T("Cached Transfer Speed"), 56 * 1024 / 8);
}

void PostFileResults::Save(ApplicationProfile *pProfile)
{
	pProfile->WriteInteger(_T("Settings"), _T("Cached Transfer Speed"), m_dwBytesPerSecond);
}
