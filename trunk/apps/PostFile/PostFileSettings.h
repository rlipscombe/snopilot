#pragma once

#include "SummaryReaderCallback.h"

class ApplicationProfile;

/** What are we posting where? */
class PostFileSettings
{
public:
	CString m_strServer;
	CString m_strObjectName;
	CString m_strLocalFilename;

	BOOL m_bAgree;

	BOOL m_bAnonymous;
	CString m_strEmailAddress;
	CString m_strForumUserName;

	SummaryReaderCallback m_summary;

public:
	void Load(ApplicationProfile *pProfile);
	void Save(ApplicationProfile *pProfile);
};

/** What happened? */
class PostFileResults
{
public:
	HRESULT m_hResult;

	DWORD m_dwStatusCode;
	CString m_strStatusText;
	CString m_strResponseHeaders;
	CString m_strResponseBody;

	// Cache the transfer speed from last time.
	DWORD m_dwBytesPerSecond;

public:
	void Load(ApplicationProfile *pProfile);
	void Save(ApplicationProfile *pProfile);
};

