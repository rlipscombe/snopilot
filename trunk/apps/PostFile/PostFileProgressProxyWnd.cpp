#include "StdAfx.h"
#include "PostFileProgressProxyWnd.h"

#define MY_WM_PROGRESS (WM_USER+42)

BEGIN_MESSAGE_MAP(CPostFileProgressProxyWnd, CWnd)
	ON_MESSAGE(MY_WM_PROGRESS, OnProgressMessage)
END_MESSAGE_MAP()

BOOL CPostFileProgressProxyWnd::Create()
{
	UINT nClassStyle = 0;
	HCURSOR hCursor = NULL;
	HBRUSH hbrBackground = NULL;
	HICON hIcon = NULL;

	LPCTSTR lpszClassName = AfxRegisterWndClass(nClassStyle, hCursor, hbrBackground, hIcon);
	return CWnd::CreateEx(0, lpszClassName, NULL, WS_POPUP, CRect(0,0,0,0), NULL, 0, NULL);
}

// Each of these methods is called on the background thread.
// We use the command pattern and a hidden window to marshal
// the calls to the foreground thread.
void CPostFileProgressProxyWnd::OnBegin()
{
	struct OnBeginCommand : public ProgressCommand
	{
		OnBeginCommand(PostFileTaskObserver *pOther) : ProgressCommand(pOther) { }
		virtual void Execute() { m_pOther->OnBegin(); }
	};

	Command *pCommand = new OnBeginCommand(m_pObserver);
	PostMessage(MY_WM_PROGRESS, 0, (LPARAM)pCommand);
}

void CPostFileProgressProxyWnd::OnCompressProgress(DWORD num, DWORD denom)
{
	struct OnUploadProgressCommand : public ProgressCommand
	{
		DWORD m_num;
		DWORD m_denom;

		OnUploadProgressCommand(PostFileTaskObserver *pOther, DWORD num, DWORD denom)
			: ProgressCommand(pOther),
				m_num(num), m_denom(denom)
		{
		}

		virtual void Execute() { m_pOther->OnCompressProgress(m_num, m_denom); }
	};

	Command *pCommand = new OnUploadProgressCommand(m_pObserver, num, denom);
	PostMessage(MY_WM_PROGRESS, 0, (LPARAM)pCommand);
}

void CPostFileProgressProxyWnd::OnResolvingName(const char *psz)
{
	struct OnResolvingNameCommand : public ProgressCommand
	{
		char *m_psz;

		OnResolvingNameCommand(PostFileTaskObserver *pOther, const char *psz)
			: ProgressCommand(pOther), m_psz(strdup(psz))
		{
		}

		~OnResolvingNameCommand()
		{
			free(m_psz);
		}

		virtual void Execute() { m_pOther->OnResolvingName(m_psz); }
	};

	Command *pCommand = new OnResolvingNameCommand(m_pObserver, psz);
	PostMessage(MY_WM_PROGRESS, 0, (LPARAM)pCommand);
}

void CPostFileProgressProxyWnd::OnResolvedName(const char *psz)
{
	struct OnResolvedNameCommand : public ProgressCommand
	{
		char *m_psz;

		OnResolvedNameCommand(PostFileTaskObserver *pOther, const char *psz)
			: ProgressCommand(pOther), m_psz(strdup(psz))
		{
		}

		~OnResolvedNameCommand()
		{
			free(m_psz);
		}

		virtual void Execute() { m_pOther->OnResolvedName(m_psz); }
	};

	Command *pCommand = new OnResolvedNameCommand(m_pObserver, psz);
	PostMessage(MY_WM_PROGRESS, 0, (LPARAM)pCommand);
}

void CPostFileProgressProxyWnd::OnConnectingToServer(const char *psz)
{
	struct OnConnectingToServerCommand : public ProgressCommand
	{
		char *m_psz;

		OnConnectingToServerCommand(PostFileTaskObserver *pOther, const char *psz)
			: ProgressCommand(pOther), m_psz(strdup(psz))
		{
		}

		~OnConnectingToServerCommand()
		{
			free(m_psz);
		}

		virtual void Execute() { m_pOther->OnConnectingToServer(m_psz); }
	};

	Command *pCommand = new OnConnectingToServerCommand(m_pObserver, psz);
	PostMessage(MY_WM_PROGRESS, 0, (LPARAM)pCommand);
}

void CPostFileProgressProxyWnd::OnConnectedToServer(const char *psz)
{
	struct OnConnectedToServerCommand : public ProgressCommand
	{
		char *m_psz;

		OnConnectedToServerCommand(PostFileTaskObserver *pOther, const char *psz)
			: ProgressCommand(pOther), m_psz(strdup(psz))
		{
		}

		~OnConnectedToServerCommand()
		{
			free(m_psz);
		}

		virtual void Execute() { m_pOther->OnConnectedToServer(m_psz); }
	};

	Command *pCommand = new OnConnectedToServerCommand(m_pObserver, psz);
	PostMessage(MY_WM_PROGRESS, 0, (LPARAM)pCommand);
}

void CPostFileProgressProxyWnd::OnSendingRequest()
{
	struct OnSendingRequestCommand : public ProgressCommand
	{
		OnSendingRequestCommand(PostFileTaskObserver *pOther) : ProgressCommand(pOther) { }
		virtual void Execute() { m_pOther->OnSendingRequest(); }
	};

	Command *pCommand = new OnSendingRequestCommand(m_pObserver);
	PostMessage(MY_WM_PROGRESS, 0, (LPARAM)pCommand);
}

void CPostFileProgressProxyWnd::OnRequestSent(DWORD dwBytesSent)
{
	struct OnRequestSentCommand : public ProgressCommand
	{
		DWORD m_dwBytesSent;

		OnRequestSentCommand(PostFileTaskObserver *pOther, DWORD dwBytesSent)
			: ProgressCommand(pOther), m_dwBytesSent(dwBytesSent)
		{
		}

		virtual void Execute() { m_pOther->OnRequestSent(m_dwBytesSent); }
	};

	Command *pCommand = new OnRequestSentCommand(m_pObserver, dwBytesSent);
	PostMessage(MY_WM_PROGRESS, 0, (LPARAM)pCommand);
}

void CPostFileProgressProxyWnd::OnReceivingResponse()
{
	struct OnReceivingResponseCommand : public ProgressCommand
	{
		OnReceivingResponseCommand(PostFileTaskObserver *pOther) : ProgressCommand(pOther) { }
		virtual void Execute() { m_pOther->OnReceivingResponse(); }
	};

	Command *pCommand = new OnReceivingResponseCommand(m_pObserver);
	PostMessage(MY_WM_PROGRESS, 0, (LPARAM)pCommand);
}

void CPostFileProgressProxyWnd::OnResponseReceived(DWORD dwBytesReceived)
{
	struct OnResponseReceivedCommand : public ProgressCommand
	{
		DWORD m_dwBytesReceived;

		OnResponseReceivedCommand(PostFileTaskObserver *pOther, DWORD dwBytesReceived)
			: ProgressCommand(pOther), m_dwBytesReceived(dwBytesReceived)
		{
		}

		virtual void Execute() { m_pOther->OnResponseReceived(m_dwBytesReceived); }
	};

	Command *pCommand = new OnResponseReceivedCommand(m_pObserver, dwBytesReceived);
	PostMessage(MY_WM_PROGRESS, 0, (LPARAM)pCommand);
}

void CPostFileProgressProxyWnd::OnUploadProgress(DWORD dwBytesWritten, DWORD dwTotalBytes, DWORD dwBytesPerSecond, DWORD dwSecondsToCompletion)
{
	struct OnUploadProgressCommand : public ProgressCommand
	{
		DWORD m_dwBytesWritten;
		DWORD m_dwTotalBytes;
		DWORD m_dwBytesPerSecond;
		DWORD m_dwSecondsToCompletion;

		OnUploadProgressCommand(PostFileTaskObserver *pOther, DWORD dwBytesWritten, DWORD dwTotalBytes,
							DWORD dwBytesPerSecond, DWORD dwSecondsToCompletion)
			: ProgressCommand(pOther),
				m_dwBytesWritten(dwBytesWritten), m_dwTotalBytes(dwTotalBytes),
				m_dwBytesPerSecond(dwBytesPerSecond), m_dwSecondsToCompletion(dwSecondsToCompletion)
		{
		}

		virtual void Execute() { m_pOther->OnUploadProgress(m_dwBytesWritten, m_dwTotalBytes, m_dwBytesPerSecond, m_dwSecondsToCompletion); }
	};

	Command *pCommand = new OnUploadProgressCommand(m_pObserver, dwBytesWritten, dwTotalBytes, dwBytesPerSecond, dwSecondsToCompletion);
	PostMessage(MY_WM_PROGRESS, 0, (LPARAM)pCommand);
}

void CPostFileProgressProxyWnd::OnComplete(HRESULT hResult)
{
	struct OnCompleteCommand : public ProgressCommand
	{
		HRESULT m_hResult;

		OnCompleteCommand(PostFileTaskObserver *pOther, HRESULT hResult)
			: ProgressCommand(pOther), m_hResult(hResult)
		{
		}

		virtual void Execute() { m_pOther->OnComplete(m_hResult); }
	};

	Command *pCommand = new OnCompleteCommand(m_pObserver, hResult);
	PostMessage(MY_WM_PROGRESS, 0, (LPARAM)pCommand);
}

bool CPostFileProgressProxyWnd::CheckCancel()
{
	return m_bCancel;
}

// The messages arrive here
LRESULT CPostFileProgressProxyWnd::OnProgressMessage(WPARAM /*wParam*/, LPARAM lParam)
{
	if (m_pObserver->CheckCancel())
		m_bCancel = true;

	Command *pCommand = reinterpret_cast<Command *>(lParam);
	pCommand->Execute();	// Not allowed to access 'this' after this point -- we may have been deleted.
	delete pCommand;

	return 0;
}
