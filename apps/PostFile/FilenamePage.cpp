// FilenamePage.cpp : implementation file
//

#include "stdafx.h"
#include "FilenamePage.h"
#include "PostFileSettings.h"
#include ".\filenamepage.h"

// CFilenamePage dialog

IMPLEMENT_DYNAMIC(CFilenamePage, CWizardPage)

CFilenamePage::CFilenamePage(PostFileSettings *pSettings)
	: CWizardPage(CFilenamePage::IDD, IDS_FORM_PAGE_CAPTION, IDS_FORM_PAGE_TITLE, IDS_FORM_PAGE_SUBTITLE),
		m_pSettings(pSettings)
{
}

CFilenamePage::~CFilenamePage()
{
}

void CFilenamePage::DoDataExchange(CDataExchange* pDX)
{
	CWizardPage::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_FILENAME, m_pSettings->m_strLocalFilename);
}


BEGIN_MESSAGE_MAP(CFilenamePage, CWizardPage)
	ON_BN_CLICKED(IDC_BROWSE, OnBnClickedBrowse)
END_MESSAGE_MAP()


// CFilenamePage message handlers

BOOL CFilenamePage::OnSetActive()
{
	if (!CWizardPage::OnSetActive())
		return FALSE;

	SetWizardButtons(PSWIZB_BACK|PSWIZB_NEXT);
	EnableCancelButton(TRUE);

	return TRUE;
}

void CFilenamePage::OnBnClickedBrowse()
{
	CFileDialog dlg(TRUE, _T("snotrack"), NULL, OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST, _T("snotrack files (*.snotrack)|*.snotrack||"));
	if (dlg.DoModal() == IDOK)
	{
		SetDlgItemText(IDC_FILENAME, dlg.GetPathName());
	}
}

LRESULT CFilenamePage::OnWizardNext()
{
	UpdateData(TRUE);
	if (!LoadStats(&m_pSettings->m_summary))
		return -1;	// Stay here.

	return CWizardPage::OnWizardNext();
}

bool CFilenamePage::LoadStats(CSnoTrackFileReaderCallback *pCallback)
{
	LPCTSTR lpszPathName = m_pSettings->m_strLocalFilename;

	CFileException fe;
	CFile *pFile = new CFile;
	ASSERT(pFile != NULL);
	if (!pFile->Open(lpszPathName,
		CFile::modeRead|CFile::shareDenyWrite, &fe))
	{
		delete pFile;
		pFile = NULL;
	}

	if (pFile == NULL)
	{
		CString message;
		message.Format(IDS_FAILED_TO_OPEN, lpszPathName);
		AfxMessageBox(message);
		return false;
	}

	TRY
	{
		CWaitCursor wait;
		CSnoTrackFileReader reader;
		reader.ReadFile(pFile, pCallback);
	}
	CATCH_ALL(e)
	{
		delete pFile;

		TRY
		{
			CString message;
			message.Format(IDS_FAILED_TO_OPEN, lpszPathName);
			AfxMessageBox(message);
		}
		END_TRY
		return false;
	}
	END_CATCH_ALL

	delete pFile;
	return true;
}

