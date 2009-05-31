#include "StdAfx.h"
#include "PostFileTask.h"
#include "PostFileSettings.h"
#include "zlib/zlib.h"
#include "file.h"
#include "FormData.h"

void PostFileTask::Run()
{
	ReportBegin();

	m_pResults->m_dwStatusCode = -1;
	HRESULT hr = PostFile(m_pSettings->m_strServer, m_pSettings->m_strObjectName, m_pSettings->m_strLocalFilename);
	m_pResults->m_hResult = hr;
	ReportComplete(hr);
}

HRESULT PostFileTask::CompressLocalFile(LPCTSTR lpszLocalFile, CString *pCompressedFileName)
{
	HANDLE hLocalFile = CreateFile(m_pSettings->m_strLocalFilename, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (hLocalFile == INVALID_HANDLE_VALUE)
		return HRESULT_FROM_WIN32(GetLastError());
	
	// So, we'll open a temporary file and write into that, instead.  Then we can send that to the server.
	std::string temporaryFileName = GetTemporaryFileName(_T("PF")) + _T(".gz");
	gzFile z = gzopen(temporaryFileName.c_str(), "wb9");
	if (!z)
	{
		CloseHandle(hLocalFile);
		return E_FAIL;
	}
	
	DWORD cbBuffer = 20480;
	char *pBuffer = (char *)malloc(cbBuffer);

	DWORD dwLocalFileSize = GetFileSize(hLocalFile, NULL);

	DWORD dwTotalBytes = 0;
	ReportCompressProgress(dwTotalBytes, dwLocalFileSize);
	HRESULT hr = S_OK;
	for (;;)
	{
		if (CheckCancel())
		{
			hr = E_USER_CANCELLED;
			break;
		}

		DWORD dwBytesRead;
		if (!ReadFile(hLocalFile, pBuffer, cbBuffer, &dwBytesRead, NULL))
		{
			hr = HRESULT_FROM_WIN32(GetLastError());
			break;
		}

		if (dwBytesRead == 0)
			break;	// End of file.

		int bytesWritten = gzwrite(z, pBuffer, dwBytesRead);
		if (bytesWritten == 0)
		{
			hr = E_FAIL;
			break;
		}

		ASSERT(bytesWritten == dwBytesRead);

		dwTotalBytes += dwBytesRead;
		ReportCompressProgress(dwTotalBytes, dwLocalFileSize);
	}

	ReportCompressProgress(dwLocalFileSize, dwLocalFileSize);

	gzclose(z);
	CloseHandle(hLocalFile);

	free(pBuffer);

	*pCompressedFileName = temporaryFileName.c_str();
	return hr;
}

class ProgressWrapper : public SendFormProgress
{
	PostFileResults *m_pResults;
	PostFileTaskObserver *m_pObserver;

public:
	ProgressWrapper(PostFileResults *pResults, PostFileTaskObserver *pObserver)
		: m_pResults(pResults), m_pObserver(pObserver)
	{
	}

	virtual void OnProgress(DWORD dwBytesWritten, DWORD dwTotalBytes, DWORD dwBytesPerSecond, DWORD dwSecondsToCompletion)
	{
		TRACE("dwBytesPerSecond = %d\n", dwBytesPerSecond);
		if (m_pObserver)
			m_pObserver->OnUploadProgress(dwBytesWritten, dwTotalBytes, dwBytesPerSecond, dwSecondsToCompletion);
		m_pResults->m_dwBytesPerSecond = dwBytesPerSecond;
	}

	virtual bool CheckCancel()
	{
		if (m_pObserver)
			return m_pObserver->CheckCancel();

		return false;
	}
};

HRESULT PostFileTask::PostFile(LPCTSTR lpszServerName, LPCTSTR lpszObjectName, LPCTSTR lpszLocalFile)
{
	// Start by compressing the local file:
	CString strCompressedFileName;
	HRESULT hr = CompressLocalFile(lpszLocalFile, &strCompressedFileName);
	if (FAILED(hr))
		return hr;

	ASSERT(strCompressedFileName.CompareNoCase(lpszLocalFile) != 0);

	LPCTSTR lpszAgent = _T("WinInetPost/0.1");
	DWORD dwOpenInternetFlags = 0;//INTERNET_FLAG_ASYNC;
	HINTERNET hInternet = InternetOpen(lpszAgent, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, dwOpenInternetFlags);
	if (!hInternet)
	{
		TRACE("InternetOpen failed, error = %d (0x%x)\n", GetLastError(), GetLastError());
		return HRESULT_FROM_WIN32(GetLastError());
	}

	HANDLE hCompressedFile = CreateFile(strCompressedFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (hCompressedFile == INVALID_HANDLE_VALUE)
	{
		TRACE("CreateFile failed, error = %d (0x%x)\n", GetLastError(), GetLastError());
		return HRESULT_FROM_WIN32(GetLastError());
	}

	InternetSetStatusCallback(hInternet, StaticInternetStatusCallback);

	INTERNET_PORT nServerPort = INTERNET_DEFAULT_HTTP_PORT;
	LPCTSTR lpszUserName = NULL;
	LPCTSTR lpszPassword = NULL;
	DWORD dwConnectFlags = 0;
	DWORD_PTR dwConnectContext = reinterpret_cast<DWORD_PTR>(this);
	HINTERNET hConnect = InternetConnect(hInternet, lpszServerName, nServerPort,
											lpszUserName, lpszPassword,
											INTERNET_SERVICE_HTTP,
											dwConnectFlags, dwConnectContext);
	if (!hConnect)
	{
		CloseHandle(hCompressedFile);
		DeleteFile(strCompressedFileName);
		InternetCloseHandle(hInternet);
		TRACE("InternetConnect failed, error = %d (0x%x)\n", GetLastError(), GetLastError());
		return HRESULT_FROM_WIN32(GetLastError());
	}

	LPCTSTR lpszVerb = _T("POST");
	LPCTSTR lpszVersion = NULL;			// Use default.
	LPCTSTR lpszReferrer = NULL;		// No referrer.
	LPCTSTR *lplpszAcceptTypes = NULL;	// Whatever the server wants to give us.
	DWORD dwOpenRequestFlags = INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTP |
								INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTPS | 
								INTERNET_FLAG_KEEP_CONNECTION |
								INTERNET_FLAG_NO_AUTH |
								INTERNET_FLAG_NO_AUTO_REDIRECT |
								INTERNET_FLAG_NO_COOKIES |
								INTERNET_FLAG_NO_UI |
								INTERNET_FLAG_RELOAD;
	DWORD_PTR dwOpenRequestContext = reinterpret_cast<DWORD_PTR>(this);
	HINTERNET hRequest = HttpOpenRequest(hConnect, lpszVerb, lpszObjectName, lpszVersion,
											lpszReferrer, lplpszAcceptTypes,
											dwOpenRequestFlags, dwOpenRequestContext);
	if (!hRequest)
	{
		CloseHandle(hCompressedFile);
		DeleteFile(strCompressedFileName);
		InternetCloseHandle(hConnect);
		InternetCloseHandle(hInternet);
		TRACE("HttpOpenRequest failed, error = %d (0x%x)\n", GetLastError(), GetLastError());
		return HRESULT_FROM_WIN32(GetLastError());
	}

	CString strFileName = lpszLocalFile;
	strFileName += ".gz";

	FormData formData;

	CString strEmailAddress;
	CString strForumUsername;
	if (m_pSettings->m_bAnonymous)
	{
		strEmailAddress = "(Anonymous)";
		strForumUsername = "(Anonymous)";
	}
	else
	{
		// If we're not anonymous, then copy the details out of the settings.
		// Otherwise, leave them blank.
		strEmailAddress = m_pSettings->m_strEmailAddress;
		strForumUsername = m_pSettings->m_strForumUserName;
	}

	formData.AddTextField("email_address", (LPCSTR)strEmailAddress);
	formData.AddTextField("forum_username", (LPCSTR)strForumUsername);
	formData.AddFileField("upload_file", (LPCSTR)strFileName, hCompressedFile);

	DWORD dwCachedBytesPerSecond = m_pResults->m_dwBytesPerSecond;
	ProgressWrapper progress(m_pResults, m_pObserver);
	hr = formData.SendForm(hRequest, dwCachedBytesPerSecond, &progress);
	if (FAILED(hr))
	{
		CloseHandle(hCompressedFile);
		DeleteFile(strCompressedFileName);
		InternetCloseHandle(hRequest);
		InternetCloseHandle(hConnect);
		InternetCloseHandle(hInternet);
		return hr;
	}

	CloseHandle(hCompressedFile);
	DeleteFile(strCompressedFileName);

	DownloadResponse(hRequest);

	InternetCloseHandle(hRequest);
	InternetCloseHandle(hConnect);
	InternetCloseHandle(hInternet);

	return S_OK;
}

void PostFileTask::DownloadResponse(HINTERNET hRequest)
{
	ReadStatusCode(hRequest, &m_pResults->m_dwStatusCode);
	ReadStatusText(hRequest, &m_pResults->m_strStatusText);
	ReadResponseHeaders(hRequest, &m_pResults->m_strResponseHeaders);
	ReadResponseBody(hRequest, &m_pResults->m_strResponseBody);
}

HRESULT PostFileTask::ReadStatusCode(HINTERNET hRequest, DWORD *pStatusCode)
{
	DWORD dwStatusCodeSize = sizeof(DWORD);
	DWORD dwStatusCode;
	if (HttpQueryInfo(hRequest, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &dwStatusCode, &dwStatusCodeSize, NULL))
		*pStatusCode = dwStatusCode;

	return S_OK;
}

HRESULT PostFileTask::ReadStatusText(HINTERNET hRequest, CString *pStatusText)
{
	HRESULT hr = S_OK;

	DWORD dwStatusTextLength = MAX_PATH;
	BYTE *pBuffer = (BYTE *)malloc(dwStatusTextLength+1);
	while (!HttpQueryInfo(hRequest, HTTP_QUERY_STATUS_TEXT, pBuffer, &dwStatusTextLength, NULL))
	{
		DWORD dwError = GetLastError();
		if (dwError == ERROR_INSUFFICIENT_BUFFER)
		{
			free(pStatusText);
			pBuffer = (BYTE *)malloc(dwStatusTextLength+1);
		}
		else
		{
			TRACE("HttpQueryInfo failed, error = %d (0x%x)\n", GetLastError(), GetLastError());
			hr = HRESULT_FROM_WIN32(GetLastError());
			break;
		}
	}

	pBuffer[dwStatusTextLength] = '\0';
	*pStatusText = pBuffer;
	free(pBuffer);

	return hr;
}

HRESULT PostFileTask::ReadResponseHeaders(HINTERNET hRequest, CString *pResponseHeaders)
{
	HRESULT hr = S_OK;

	DWORD dwInfoLevel = HTTP_QUERY_RAW_HEADERS_CRLF;
	DWORD dwInfoBufferLength = 1024;
	BYTE *pInfoBuffer = (BYTE *)malloc(dwInfoBufferLength+1);
	while (!HttpQueryInfo(hRequest, dwInfoLevel, pInfoBuffer, &dwInfoBufferLength, NULL))
	{
		DWORD dwError = GetLastError();
		if (dwError == ERROR_INSUFFICIENT_BUFFER)
		{
			free(pInfoBuffer);
			pInfoBuffer = (BYTE *)malloc(dwInfoBufferLength+1);
		}
		else
		{
			TRACE("HttpQueryInfo failed, error = %d (0x%x)\n", GetLastError(), GetLastError());
			hr = HRESULT_FROM_WIN32(GetLastError());
		}
	}

	pInfoBuffer[dwInfoBufferLength] = '\0';
	*pResponseHeaders = pInfoBuffer;
	free(pInfoBuffer);

	return hr;
}

HRESULT PostFileTask::ReadResponseBody(HINTERNET hRequest, CString *pResponseBody)
{
	HRESULT hr = S_OK;

	pResponseBody->Empty();

	DWORD dwBytesAvailable;
	while (InternetQueryDataAvailable(hRequest, &dwBytesAvailable, 0, 0))
	{
		if (dwBytesAvailable == 0)
			break;	// End of file.

		BYTE *pMessageBody = (BYTE *)malloc(dwBytesAvailable+1);

		DWORD dwBytesRead;
		BOOL bResult = InternetReadFile(hRequest, pMessageBody, dwBytesAvailable, &dwBytesRead);
		if (!bResult)
		{
			TRACE("InternetReadFile failed, error = %d (0x%x)\n", GetLastError(), GetLastError());
			hr = HRESULT_FROM_WIN32(GetLastError());
		}

		if (dwBytesRead == 0)
		{
			free(pMessageBody);
			break;	// End of File.
		}

		pMessageBody[dwBytesRead] = '\0';
		pResponseBody->Append(LPCTSTR(pMessageBody));
		free(pMessageBody);
	}

	return hr;
}

/* static */
void CALLBACK PostFileTask::StaticInternetStatusCallback(HINTERNET hInternet, DWORD_PTR dwContext, DWORD dwInternetStatus, LPVOID lpvStatusInformation, DWORD dwStatusInformationLength)
{
	PostFileTask *pThis = reinterpret_cast<PostFileTask *>(dwContext);
	pThis->InternetStatusCallback(hInternet, dwContext, dwInternetStatus, lpvStatusInformation, dwStatusInformationLength);
}

void PostFileTask::InternetStatusCallback(HINTERNET hInternet, DWORD_PTR dwContext, DWORD dwInternetStatus, LPVOID lpvStatusInformation, DWORD dwStatusInformationLength)
{
	TRACE("InternetStatusCallback(hInternet = %p, dwInternetStatus = %d, lpvStatusInformation = %p, dwStatusInformationLength = %d)\n",
				hInternet, dwInternetStatus, lpvStatusInformation, dwStatusInformationLength);

	if (!m_pObserver)
		return;	// Don't bother.

	switch (dwInternetStatus)
	{
	case INTERNET_STATUS_RESOLVING_NAME:
		// lpvStatusInformation is the server name.
		m_pObserver->OnResolvingName((const char *)lpvStatusInformation);
		break;
	case INTERNET_STATUS_NAME_RESOLVED:
		m_pObserver->OnResolvedName((const char *)lpvStatusInformation);
		break;
	case INTERNET_STATUS_CONNECTING_TO_SERVER:
		// The documentation says that lpvStatusInformation is a SOCKADDR structure.  It lies.
		m_pObserver->OnConnectingToServer((const char *)lpvStatusInformation);
		break;
	case INTERNET_STATUS_CONNECTED_TO_SERVER:
		// The documentation says that lpvStatusInformation is a SOCKADDR structure.  It lies.
		m_pObserver->OnConnectedToServer((const char *)lpvStatusInformation);
		break;
	case INTERNET_STATUS_SENDING_REQUEST:
		// lpvStatusInformation is NULL.
		m_pObserver->OnSendingRequest();
		break;
	case INTERNET_STATUS_REQUEST_SENT:
		{
			DWORD dwBytesSent = *(DWORD *)lpvStatusInformation;
			m_pObserver->OnRequestSent(dwBytesSent);
		}
		break;
	case INTERNET_STATUS_RECEIVING_RESPONSE:
		// lpvStatusInformation is NULL.
		m_pObserver->OnReceivingResponse();
		break;
	case INTERNET_STATUS_RESPONSE_RECEIVED:
		{
			DWORD dwBytesReceived = *(DWORD *)lpvStatusInformation;
			m_pObserver->OnResponseReceived(dwBytesReceived);
		}
		break;
	case INTERNET_STATUS_CTL_RESPONSE_RECEIVED:
		TRACE("INTERNET_STATUS_CTL_RESPONSE_RECEIVED: Not implemented.\n");
		break;
	case INTERNET_STATUS_PREFETCH:
		TRACE("INTERNET_STATUS_PREFETCH:\n");
		break;
	case INTERNET_STATUS_CLOSING_CONNECTION:
		// lpvStatusInformation is NULL.
		TRACE("INTERNET_STATUS_CLOSING_CONNECTION: Closing the connection to the server.\n");
		break;
	case INTERNET_STATUS_CONNECTION_CLOSED:
		// lpvStatusInformation is NULL.
		TRACE("INTERNET_STATUS_CONNECTION_CLOSED: Successfully closed the connection to the server.\n");
		break;
	case INTERNET_STATUS_HANDLE_CREATED:
		{
			HINTERNET hCreated = *(HINTERNET *)lpvStatusInformation;
			TRACE("INTERNET_STATUS_HANDLE_CREATED: %p\n", hCreated);
		}
		break;
	case INTERNET_STATUS_HANDLE_CLOSING:
		{
			HINTERNET hClosing = *(HINTERNET *)lpvStatusInformation;
			TRACE("INTERNET_STATUS_HANDLE_CLOSING: %p\n", hClosing);
		}
		break;
	case INTERNET_STATUS_DETECTING_PROXY:
		TRACE("INTERNET_STATUS_DETECTING_PROXY: Notifies the client application that a proxy has been detected.\n");
		break;
	case INTERNET_STATUS_REQUEST_COMPLETE:
		TRACE("INTERNET_STATUS_REQUEST_COMPLETE:\n");
		break;
	case INTERNET_STATUS_REDIRECT:
		TRACE("INTERNET_STATUS_REDIRECT:\n");
		break;
	case INTERNET_STATUS_INTERMEDIATE_RESPONSE:
		TRACE("INTERNET_STATUS_INTERMEDIATE_RESPONSE:\n");
		break;
	case INTERNET_STATUS_USER_INPUT_REQUIRED:
		TRACE("INTERNET_STATUS_USER_INPUT_REQUIRED:\n");
		break;
	case INTERNET_STATUS_STATE_CHANGE:
		TRACE("INTERNET_STATUS_STATE_CHANGE:\n");
		break;
	case INTERNET_STATUS_COOKIE_SENT:
		TRACE("INTERNET_STATUS_COOKIE_SENT:\n");
		break;
	case INTERNET_STATUS_COOKIE_RECEIVED:
		TRACE("INTERNET_STATUS_COOKIE_RECEIVED:\n");
		break;
	case INTERNET_STATUS_PRIVACY_IMPACTED:
		TRACE("INTERNET_STATUS_PRIVACY_IMPACTED:\n");
		break;
	case INTERNET_STATUS_P3P_HEADER:
		TRACE("INTERNET_STATUS_P3P_HEADER:\n");
		break;
	case INTERNET_STATUS_P3P_POLICYREF:
		TRACE("INTERNET_STATUS_P3P_POLICYREF:\n");
		break;
	case INTERNET_STATUS_COOKIE_HISTORY:
		TRACE("INTERNET_STATUS_COOKIE_HISTORY:\n");
		break;
	}
}

