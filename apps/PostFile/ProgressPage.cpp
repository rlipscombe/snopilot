// ProgressPage.cpp : implementation file
//

#include "stdafx.h"
#include "ProgressPage.h"
#include "PostFileTaskThread.h"
#include "PostFileSettings.h"

// CProgressPage dialog
IMPLEMENT_DYNAMIC(CProgressPage, CWizardPage)

CProgressPage::CProgressPage(PostFileSettings *pSettings, PostFileResults *pResults)
	: CWizardPage(CProgressPage::IDD, IDS_PROGRESS_PAGE_CAPTION, IDS_PROGRESS_PAGE_TITLE, IDS_PROGRESS_PAGE_SUBTITLE),
		m_pProxyWnd(NULL), m_pThread(NULL), m_pSettings(pSettings), m_pResults(pResults)
{
}

CProgressPage::~CProgressPage()
{
}

void CProgressPage::DoDataExchange(CDataExchange* pDX)
{
	CWizardPage::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_PROGRESSCTRL, m_progressCtrl);
}

BEGIN_MESSAGE_MAP(CProgressPage, CWizardPage)
END_MESSAGE_MAP()

// CProgressPage message handlers
BOOL CProgressPage::OnSetActive()
{
	if (!CWizardPage::OnSetActive())
		return FALSE;

	// Because the property page isn't fully created and displayed at this point,
	// we need to use PostMessage to defer the long-lasting task for a short while.
	
	// First, we'll disable the buttons, to prevent the user from pressing anything before we get going.
	SetWizardButtons(0);
	EnableCancelButton(FALSE);

	m_bCancel = false;

	ASSERT(!m_pProxyWnd);
	m_pProxyWnd = new CPostFileProgressProxyWnd(this);
	VERIFY(m_pProxyWnd->Create());

	ASSERT(!m_pThread);
	m_pThread = new PostFileTaskThread(m_pSettings, m_pResults, m_pProxyWnd);
	m_pThread->Start();

	return TRUE;
}

void CProgressPage::OnBegin()
{
	EnableCancelButton(TRUE);
}

void CProgressPage::OnResolvingName(const char *psz)
{
	CString status;
	status.LoadString(IDS_RESOLVING_NAME);
	SetDlgItemText(IDC_STATUS, status);
}

void CProgressPage::OnResolvedName(const char *psz)
{
	CString status;
	status.LoadString(IDS_RESOLVED_NAME);
	SetDlgItemText(IDC_STATUS, status);
}

void CProgressPage::OnConnectingToServer(const char *psz)
{
	CString status;
	status.LoadString(IDS_CONNECTING_TO_SERVER);
	SetDlgItemText(IDC_STATUS, status);
}

void CProgressPage::OnConnectedToServer(const char *psz)
{
	CString status;
	status.LoadString(IDS_CONNECTED_TO_SERVER);
	SetDlgItemText(IDC_STATUS, status);
}

void CProgressPage::OnSendingRequest()
{
}

void CProgressPage::OnRequestSent(DWORD dwBytesSent)
{
}

void CProgressPage::OnReceivingResponse()
{
	CString status;
	status.LoadString(IDS_RECEIVING_RESPONSE);
	SetDlgItemText(IDC_STATUS, status);
}

void CProgressPage::OnResponseReceived(DWORD dwBytesReceived)
{
	CString status;
	status.LoadString(IDS_RECEIVED_RESPONSE);
	SetDlgItemText(IDC_STATUS, status);
}

CString FormatTransferSpeed(DWORD dwBytesPerSecond)
{
	CString s;
	if (dwBytesPerSecond < 1024)
	{
		// Less than 1KB/sec, so...
		s.Format(IDS_TRANSFER_SPEED_BYTES_PER_SEC, dwBytesPerSecond);
	}
	else if (dwBytesPerSecond < 1024 * 1024)
	{
		// Less than 1MB/sec, so...
		s.Format(IDS_TRANSFER_SPEED_KB_PER_SEC, ((double)dwBytesPerSecond) / 1024.0);
	}
	else
	{
		// Something else, so...
		s.Format(IDS_TRANSFER_SPEED_MB_PER_SEC, ((double)dwBytesPerSecond) / (1024.0 * 1024.0));
	}
	return s;
}

CString FormatTimeRemaining(DWORD dwSecondsRemaining)
{
	TCHAR buffer[MAX_PATH];
	StrFromTimeInterval(buffer, COUNTOF(buffer), dwSecondsRemaining * 1000, 3);
	return buffer;
}

void CProgressPage::OnCompressProgress(DWORD num, DWORD denom)
{
	m_progressCtrl.SetRange32(0, denom);
	m_progressCtrl.SetPos(num);

	CString status;
	status.Format(IDS_COMPRESS_PROGRESS, MulDiv(100, num, denom));

	CString old;
	GetDlgItemText(IDC_STATUS, old);
	
	if (old != status)
		SetDlgItemText(IDC_STATUS, status);
}

void CProgressPage::OnUploadProgress(DWORD dwBytesWritten, DWORD dwTotalBytes, DWORD dwBytesPerSecond, DWORD dwSecondsToCompletion)
{
	m_progressCtrl.SetRange32(0, dwTotalBytes);
	m_progressCtrl.SetPos(dwBytesWritten);

	CString transferSpeed = FormatTransferSpeed(dwBytesPerSecond);
	CString secondsRemaining = FormatTimeRemaining(dwSecondsToCompletion);

	CString status;
	status.Format(IDS_UPLOAD_PROGRESS, LPCTSTR(transferSpeed), LPCTSTR(secondsRemaining));
	SetDlgItemText(IDC_STATUS, status);

	if (GetParent()->IsIconic())
	{
		CString caption;
		caption.Format(IDS_UPLOAD_PROGRESS_PERCENT, MulDiv(100, dwBytesWritten, dwTotalBytes));
		GetParent()->SetWindowText(caption);
	}
	else
		SetDefaultCaption();
}

void CProgressPage::SetDefaultCaption()
{
	CString caption;
	caption.LoadString(IDS_UPLOADING_FILE_CAPTION);
	GetParent()->SetWindowText(caption);
}

void CProgressPage::OnComplete(HRESULT hResult)
{
	SetDefaultCaption();

	// Wait for the thread to finish before deleting it:
	m_pThread->Join();
	delete m_pThread;
	m_pThread = NULL;

	// Once the background thread's finished, it's safe to destroy the proxy window:
	m_pProxyWnd->DestroyWindow();
	delete m_pProxyWnd;
	m_pProxyWnd = NULL;

	static_cast<CPropertySheet *>(GetParent())->PressButton(PSBTN_NEXT);
}

/** Called when the Cancel button is pressed.
 * Return FALSE to prevent cancellation.
 * Remember that the button was pressed for CheckCancelButton.
 */
BOOL CProgressPage::OnQueryCancel()
{
	// Disable the Cancel button, to give the user feedback.
	EnableCancelButton(FALSE);

	m_bCancel = true;
	return FALSE;
}

bool CProgressPage::CheckCancel()
{
	return m_bCancel;
}
