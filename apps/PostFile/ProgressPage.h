#pragma once

#include "resource.h"
#include "win32_ui/WizardPage.h"
#include "PostFileTask.h"
#include "PostFileProgressProxyWnd.h"

class PostFileSettings;
class PostFileResults;

class PostFileTaskThread;

class CPostFileProgressProxyWnd;

// CProgressPage dialog
class CProgressPage : public CWizardPage, private PostFileTaskObserver
{
	CPostFileProgressProxyWnd *m_pProxyWnd;
	PostFileTaskThread *m_pThread;
	PostFileSettings *m_pSettings;
	PostFileResults *m_pResults;
	bool m_bCancel;

	DECLARE_DYNAMIC(CProgressPage)

public:
	CProgressPage(PostFileSettings *pSettings, PostFileResults *pResults);
	virtual ~CProgressPage();

	virtual BOOL OnSetActive();
	virtual BOOL OnQueryCancel();

// Dialog Data
	enum { IDD = IDD_PROGRESS_PAGE };
	CProgressCtrl m_progressCtrl;

// PostFileTaskObserver
private:
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

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	void SetDefaultCaption();
};
