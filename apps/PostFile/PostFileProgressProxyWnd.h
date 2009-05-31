#pragma once

#include "PostFileTask.h"

/** Fairly standard marshalling of progress by using a hidden window.
 */
class CPostFileProgressProxyWnd : public CWnd, public PostFileTaskObserver
{
	PostFileTaskObserver *m_pObserver;
	bool m_bCancel;

public:
	CPostFileProgressProxyWnd(PostFileTaskObserver *pObserver)
		: m_pObserver(pObserver), m_bCancel(false)
	{
	}

	BOOL Create();

// PostFileTaskObserver
public:
	virtual void OnBegin();

	virtual void OnCompressProgress(DWORD num, DWORD denom);

	virtual void OnResolvingName(const char *psz);
	virtual void OnResolvedName(const char *psz);

	virtual void OnConnectingToServer(const char *psz);
	virtual void OnConnectedToServer(const char *psz);

	virtual void OnSendingRequest();
	virtual void OnRequestSent(DWORD dwBytesSent);

	virtual void OnReceivingResponse();
	virtual void OnResponseReceived(DWORD dwBytesReceived);

	virtual void OnUploadProgress(DWORD dwBytesWritten, DWORD dwTotalBytes, DWORD dwBytesPerSecond, DWORD dwSecondsToCompletion);
	virtual void OnComplete(HRESULT hResult);

	virtual bool CheckCancel();

public:
	DECLARE_MESSAGE_MAP()

	afx_msg LRESULT OnProgressMessage(WPARAM wParam, LPARAM lParam);
};

class Command
{
public:
	virtual ~Command()
	{
	}

	virtual void Execute() = 0;
};

class ProgressCommand : public Command
{
protected:
	PostFileTaskObserver *m_pOther;

	ProgressCommand(PostFileTaskObserver *pOther)
		: m_pOther(pOther)
	{
	}
};
