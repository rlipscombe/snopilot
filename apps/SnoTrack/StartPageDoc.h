#pragma once


// CStartPageDoc document

class CStartPageDoc : public CDocument
{
	DECLARE_DYNCREATE(CStartPageDoc)

public:
	CStartPageDoc();
	virtual ~CStartPageDoc();
	virtual void Serialize(CArchive& ar);   // overridden for document i/o
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	virtual BOOL OnNewDocument();

	DECLARE_MESSAGE_MAP()
};
