#pragma once

#include <string>

class PostFileSettings;
class PostFileResults;
class PostFileTaskObserver;

class FormData;

class PostFileTask
{
	PostFileSettings *m_pSettings;
	PostFileResults *m_pResults;
	PostFileTaskObserver *m_pObserver;

public:
	PostFileTask(PostFileSettings *pSettings, PostFileResults *pResults, PostFileTaskObserver *pObserver)
		: m_pSettings(pSettings), m_pResults(pResults), m_pObserver(pObserver)
	{
	}

	void Run();

private:
	void ReportBegin();
	void ReportCompressProgress(DWORD num, DWORD denom);
	void ReportUploadProgress(DWORD dwBytesWritten, DWORD dwTotalBytes, DWORD dwBytesPerSecond, DWORD dwSecondsToCompletion);
	void ReportComplete(HRESULT hResult);
	bool CheckCancel();

private:
	static void CALLBACK StaticInternetStatusCallback(HINTERNET hInternet, DWORD_PTR dwContext,
		DWORD dwInternetStatus, LPVOID lpvStatusInformation, DWORD dwStatusInformationLength);
	void CALLBACK InternetStatusCallback(HINTERNET hInternet, DWORD_PTR dwContext,
		DWORD dwInternetStatus, LPVOID lpvStatusInformation, DWORD dwStatusInformationLength);

	HRESULT PostFile(LPCTSTR lpszServerName, LPCTSTR lpszObjectName, LPCTSTR lpszLocalFile);

	HRESULT CompressLocalFile(LPCTSTR lpszLocalFile, CString *pCompressedFileName);

	HRESULT UploadFormData(HINTERNET hRequest, const FormData &formData);
	HRESULT UploadFileContents(HANDLE hLocalFile, HINTERNET hRemoteFile);

	void DownloadResponse(HINTERNET hRequest);
	HRESULT ReadStatusCode(HINTERNET hRequest, DWORD *pStatusCode);
	HRESULT ReadStatusText(HINTERNET hRequest, CString *pStatusText);
	HRESULT ReadResponseHeaders(HINTERNET hRequest, CString *pResponseHeaders);
	HRESULT ReadResponseBody(HINTERNET hRequest, CString *pResponseBody);
};

class PostFileTaskObserver
{
public:
	virtual void OnBegin() = 0;

	virtual void OnCompressProgress(DWORD num, DWORD denom) = 0;

	virtual void OnResolvingName(const char *psz) = 0;
	virtual void OnResolvedName(const char *psz) = 0;

	virtual void OnConnectingToServer(const char *psz) = 0;
	virtual void OnConnectedToServer(const char *psz) = 0;

	virtual void OnSendingRequest() = 0;
	virtual void OnRequestSent(DWORD dwBytesSent) = 0;

	virtual void OnReceivingResponse() = 0;
	virtual void OnResponseReceived(DWORD dwBytesReceived) = 0;

	virtual void OnUploadProgress(DWORD dwBytesWritten, DWORD dwTotalBytes, DWORD dwBytesPerSecond, DWORD dwSecondsToCompletion) = 0;

	virtual void OnComplete(HRESULT hResult) = 0;

	virtual bool CheckCancel() = 0;
};

inline void PostFileTask::ReportBegin()
{
	if (m_pObserver)
		m_pObserver->OnBegin();
}

inline void PostFileTask::ReportComplete(HRESULT hResult)
{
	if (m_pObserver)
		m_pObserver->OnComplete(hResult);
}

inline void PostFileTask::ReportCompressProgress(DWORD num, DWORD denom)
{
	if (m_pObserver)
		m_pObserver->OnCompressProgress(num, denom);
}

inline void PostFileTask::ReportUploadProgress(DWORD dwBytesWritten, DWORD dwTotalBytes, DWORD dwBytesPerSecond, DWORD dwSecondsToCompletion)
{
	if (m_pObserver)
		m_pObserver->OnUploadProgress(dwBytesWritten, dwTotalBytes, dwBytesPerSecond, dwSecondsToCompletion);
}

inline bool PostFileTask::CheckCancel()
{
	if (m_pObserver)
		return m_pObserver->CheckCancel();

	// Keep going.
	return false;
}
