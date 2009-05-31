// StartPageDoc.cpp : implementation file
//

#include "stdafx.h"
#include "StartPageDoc.h"
#include "resource.h"

// CStartPageDoc

IMPLEMENT_DYNCREATE(CStartPageDoc, CDocument)

CStartPageDoc::CStartPageDoc()
{
}

BOOL CStartPageDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	CString title;
	title.LoadString(IDS_START_PAGE_TITLE);
	SetTitle(title);
	return TRUE;
}

CStartPageDoc::~CStartPageDoc()
{
}


BEGIN_MESSAGE_MAP(CStartPageDoc, CDocument)
END_MESSAGE_MAP()


// CStartPageDoc diagnostics

#ifdef _DEBUG
void CStartPageDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CStartPageDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CStartPageDoc serialization

void CStartPageDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}


// CStartPageDoc commands
