// StartPageView.cpp : implementation file
//

#include "stdafx.h"
#include "StartPageView.h"
#include "SnoTrack.h"
#include "win32_ui/bold_font.h"

// CStartPageView

IMPLEMENT_DYNCREATE(CStartPageView, CFormView)

CStartPageView::CStartPageView()
	: CFormView(CStartPageView::IDD), m_pRecentFileList(NULL)
{
}

CStartPageView::~CStartPageView()
{
}

void CStartPageView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LISTCTRL, m_listCtrl);
	DDX_Control(pDX, IDC_NEW_FILE, m_newFileButton);
	DDX_Control(pDX, IDC_OPEN_FILE, m_openFileButton);
}

BEGIN_MESSAGE_MAP(CStartPageView, CFormView)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE, OnUpdateFileSave)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE_AS, OnUpdateFileSaveAs)
	ON_UPDATE_COMMAND_UI(ID_WINDOW_NEW, OnUpdateWindowNew)
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_NEW_FILE, OnBnClickedNewFile)
	ON_BN_CLICKED(IDC_OPEN_FILE, OnBnClickedOpenFile)
	ON_WM_CTLCOLOR()
	ON_NOTIFY(LVN_ITEMACTIVATE, IDC_LISTCTRL, OnLvnItemActivateListctrl)
	ON_NOTIFY(LVN_GETINFOTIP, IDC_LISTCTRL, OnLvnGetInfoTipListctrl)
	ON_WM_ACTIVATE()
END_MESSAGE_MAP()

// CStartPageView diagnostics

#ifdef _DEBUG
void CStartPageView::AssertValid() const
{
	CFormView::AssertValid();
}

void CStartPageView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif //_DEBUG

// CStartPageView message handlers
void CStartPageView::OnUpdateFileSave(CCmdUI *pCmdUI)
{
	// You can't save the start page.
	pCmdUI->Enable(FALSE);
}

void CStartPageView::OnUpdateFileSaveAs(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(FALSE);
}

void CStartPageView::OnUpdateWindowNew(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(FALSE);
}

void CStartPageView::OnDraw(CDC *pDC)
{
	CRect clientRect;
	GetClientRect(&clientRect);
	pDC->FillSolidRect(clientRect, GetSysColor(COLOR_WINDOW));
}

BOOL CStartPageView::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}

void CStartPageView::OnSize(UINT nType, int cx, int cy)
{
	RecalcLayout();
}

void CStartPageView::RecalcLayout()
{
	CRect clientRect;
	GetClientRect(&clientRect);

	CRect listBoxRect(clientRect);
	listBoxRect.DeflateRect(10, 0, 10, 0);
	listBoxRect.right = listBoxRect.left + 500;

	CWnd *pTitle = GetDlgItem(IDC_TITLE);
	if (pTitle)
	{
		CRect r;
		pTitle->GetWindowRect(&r);

		pTitle->SetWindowPos(NULL, 10, 10, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
		
		listBoxRect.top += r.Height() + 12;
	}

	CWnd *pNewFileButton = GetDlgItem(IDC_NEW_FILE);
	CWnd *pOpenFileButton = GetDlgItem(IDC_OPEN_FILE);

	if (pNewFileButton && pOpenFileButton)
	{
		CRect r;
		pNewFileButton->GetWindowRect(&r);
		CSize newFileButtonSize = r.BottomRight() - r.TopLeft();

		pOpenFileButton->GetWindowRect(&r);
		CSize openFileButtonSize = r.BottomRight() - r.TopLeft();

		CRect newFileButtonRect(CPoint(0, 0), newFileButtonSize);
		CRect openFileButtonRect(CPoint(0, 0), openFileButtonSize);
		newFileButtonRect.OffsetRect(10, clientRect.Height() - r.Height() - 40);
		openFileButtonRect.OffsetRect(newFileButtonRect.right + 10, clientRect.Height() - r.Height() - 40);

		// Move both buttons down there.
		pNewFileButton->SetWindowPos(NULL, newFileButtonRect.left, newFileButtonRect.top,
			newFileButtonRect.Width(), newFileButtonRect.Height(), SWP_NOZORDER);
		pOpenFileButton->SetWindowPos(NULL, openFileButtonRect.left, openFileButtonRect.top,
			openFileButtonRect.Width(), openFileButtonRect.Height(), SWP_NOZORDER);

		listBoxRect.bottom = newFileButtonRect.top - 10;
	}

	if (m_listCtrl.GetSafeHwnd())
		m_listCtrl.SetWindowPos(NULL, listBoxRect.left, listBoxRect.top, listBoxRect.Width(), listBoxRect.Height(), SWP_NOZORDER);
}

void CStartPageView::OnBnClickedNewFile()
{
	AfxGetMainWnd()->SendMessage(WM_COMMAND, ID_FILE_NEW);
}

void CStartPageView::OnBnClickedOpenFile()
{
	AfxGetMainWnd()->SendMessage(WM_COMMAND, ID_FILE_OPEN);
}

void CStartPageView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	m_pRecentFileList = static_cast<CSnoTrackApp *>(AfxGetApp())->GetRecentFileList();

	LOGFONT lf;
	GetMessageBoxFont(&lf);

	CDC *pDC = GetDC();
	lf.lfHeight = -MulDiv(11, GetDeviceCaps(pDC->m_hDC, LOGPIXELSY), 72);
	ReleaseDC(pDC);

	lf.lfWeight = FW_BOLD;

	m_boldFont.CreateFontIndirect(&lf);

	GetDlgItem(IDC_TITLE)->SetFont(&m_boldFont);

	// Tweak the styles of the list box.
	m_listCtrl.SetExtendedStyle(m_listCtrl.GetExtendedStyle() | LVS_EX_GRIDLINES | LVS_EX_INFOTIP | LVS_EX_ONECLICKACTIVATE | LVS_EX_UNDERLINEHOT);

	m_listCtrl.InsertColumn(NEW NameColumn());

	PopulateListCtrl();
}

void CStartPageView::PopulateListCtrl()
{
	if (!m_pRecentFileList)
		return;

	m_listCtrl.DeleteAllItems();

	TCHAR szCurDir[_MAX_PATH];
	DWORD dwDirLen = GetCurrentDirectory(_MAX_PATH, szCurDir);
	if( dwDirLen == 0 || dwDirLen >= _MAX_PATH )
		return;	// Path too long

	int nCurDir = lstrlen(szCurDir);
	ASSERT(nCurDir >= 0);
	szCurDir[nCurDir] = '\\';
	szCurDir[++nCurDir] = '\0';

//	TRACE("Recent File List contains %d item(s)\n", m_pRecentFileList->GetSize());
	for (int i = 0; i < m_pRecentFileList->GetSize(); ++i)
	{
		CString strFileName;
		if (!m_pRecentFileList->GetDisplayName(strFileName, i, szCurDir, nCurDir))
			break;

		// Get the file's full path.
		CString strFullPath = m_pRecentFileList->m_arrNames[i];
//		TRACE(" %s (%s)\n", (LPCTSTR)strFileName, (LPCTSTR)strFullPath);

		/// @todo Open the file to get the other details from it
		CFileStatus fileStatus;
		if (CFile::GetStatus(strFullPath, fileStatus))
			m_listCtrl.InsertItem(new CStartPageItem(strFileName, strFullPath));
	}
}

HBRUSH CStartPageView::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CFormView::OnCtlColor(pDC, pWnd, nCtlColor);

	if (nCtlColor == CTLCOLOR_STATIC && pWnd->GetDlgCtrlID() == IDC_TITLE)
	{
		pDC->SetBkColor(GetSysColor(COLOR_WINDOW));

		return NULL;
	}

	return hbr;
}

void CStartPageView::OnLvnItemActivateListctrl(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMIA = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);

	const CStartPageItem *pItem = static_cast<const CStartPageItem *>(m_listCtrl.GetItem(pNMIA->iItem));
	if (pItem)
	{
		tstring full_path = pItem->GetFullPath();
		AfxGetApp()->OpenDocumentFile(full_path.c_str());
	}

	*pResult = 0;
}

void CStartPageView::OnLvnGetInfoTipListctrl(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLVGETINFOTIP *pGetInfoTip = reinterpret_cast<NMLVGETINFOTIP *>(pNMHDR);

	const CStartPageItem *pItem = static_cast<const CStartPageItem *>(m_listCtrl.GetItem(pGetInfoTip->iItem));
	if (pItem)
	{
		tstring full_path = pItem->GetFullPath();
		lstrcpyn(pGetInfoTip->pszText, full_path.c_str(), pGetInfoTip->cchTextMax);
	}
	
	*pResult = 0;
}

tstring NameColumn::GetText(const CListCtrlItem *pItem) const
{
	return static_cast<const CStartPageItem *>(pItem)->GetName();
}

int NameColumn::Compare(const CListCtrlItem *lhs, const CListCtrlItem *rhs) const
{
	tstring lhs_text = GetText(lhs);
	tstring rhs_text = GetText(rhs);

	return _tcsicmp(lhs_text.c_str(), rhs_text.c_str());
}

void CStartPageView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView)
{
	if (bActivate)
		PopulateListCtrl();

	CFormView::OnActivateView(bActivate, pActivateView, pDeactiveView);
}
