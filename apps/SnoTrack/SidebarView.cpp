// SidebarView.cpp : implementation file
//

#include "stdafx.h"
#include "SidebarView.h"
#include "SnoTrackDoc.h"
#include "HotspotVisit.h"
#include "win32/TimeZoneInformation.h"
#include "ChildFrm.h"
#include "CartesianView.h"
#include "CalcRoute.h"
#include ".\sidebarview.h"

// CSidebarView

IMPLEMENT_DYNCREATE(CSidebarView, CFormView)

CSidebarView::CSidebarView()
	: CFormView(CSidebarView::IDD)
{
}

CSidebarView::~CSidebarView()
{
}

void CSidebarView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LISTBOX, m_listBox);
}

BEGIN_MESSAGE_MAP(CSidebarView, CFormView)
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_LBN_SELCHANGE(IDC_LISTBOX, OnLbnSelchangeListbox)
END_MESSAGE_MAP()


// CSidebarView diagnostics

#ifdef _DEBUG
void CSidebarView::AssertValid() const
{
	CFormView::AssertValid();
}

void CSidebarView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif //_DEBUG


// CSidebarView message handlers
void CSidebarView::OnUpdate(CView* /*pSender*/, LPARAM lHint, CObject* /*pHint*/)
{
	// Do we need to re-run the match-paths algorithm?
	if (lHint & HINT_EDIT_RESORT_PATHS|HINT_EDIT_RESORT_HOTSPOTS|HINT_IMPORT_TRACKLOG)
	{
		CSnoTrackDoc* pDoc = (CSnoTrackDoc*)GetDocument();
		ASSERT_VALID(pDoc);
		if (!pDoc)
			return;

		CWaitCursor busy;

		TIME_ZONE_INFORMATION tzi;
		GetRegistryTimeZoneInformation(pDoc->GetTimeZoneName().c_str(), &tzi);

		m_listBox.ResetContent();

		const RouteEventCollection &route = pDoc->GetRoute();
		for (RouteEventCollection::const_iterator i = route.begin(); i != route.end(); ++i)
			m_listBox.AddEvent(i->description, i->startTime, i->endTime, i->path ? i->path->GetDistance() : 0, i->imageIndex);
	}
}

void CSidebarView::OnSize(UINT nType, int cx, int cy)
{
	if (m_listBox.GetSafeHwnd())
		m_listBox.SetWindowPos(NULL, 0, 0, cx, cy, SWP_NOZORDER);

	// don't: CFormView::OnSize(nType, cx, cy);
}

void CSidebarView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	CImageList *piml = new CImageList;
	VERIFY(piml->Create(IDB_EVENT_ICONS, 16, 1, RGB(255,0,255)));
	m_listBox.SetImageList(piml);

	CSnoTrackDoc* pDoc = (CSnoTrackDoc*)GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	TIME_ZONE_INFORMATION tzi;
	GetRegistryTimeZoneInformation(pDoc->GetTimeZoneName().c_str(), &tzi);

	m_listBox.SetTimeZoneInformation(&tzi);
}

void CSidebarView::OnDestroy()
{
	delete m_listBox.SetImageList(NULL);

	CFormView::OnDestroy();
}

void CSidebarView::OnLbnSelchangeListbox()
{
	// Hmmm.  We need to tell the CCartesianView to highlight these items without
	// knowing how it's going to do it.

	// We need to hand it a set of (start,end) tuples and it's going to select the relevant parts of
	// the tracklog.

	// First up: how do we get a pointer to the CCartesianView?
	CChildFrame *pFrame = static_cast<CChildFrame *>(GetParentFrame());

	// Get hold of the cartesian view.
	CCartesianView *pView = pFrame->GetCartesianView();
	
	// In order for us to have a selection, we need paths/hotspots.
	// If there are paths/hotspots, there must be data.
	// If there's data, there must be a CCartesianView.
	ASSERT(pView != NULL);
	if (!pView)
		return;	// But we'll fail-safe anyway.

	// The selection in the list box has changed.
	// We ought to find out which items are now selected.
	int count = m_listBox.GetSelCount();
	int *items = new int[count];

	SelectionTimeSpanCollection spans;

	count = m_listBox.GetSelItems(count, items);
	for (int i = 0; i < count; ++i)
	{
		int nIndex = items[i];

		FILETIME entryTime, exitTime;
		VERIFY(m_listBox.GetEvent(nIndex, &entryTime, &exitTime));

		spans.push_back(SelectionTimeSpan(entryTime, exitTime));
	}

	delete [] items;

	pView->SelectTimeSpans(spans);
}

BOOL CSidebarView::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// Throw it at the Cartesian view first:
	if (nCode == CN_COMMAND)
	{
		CChildFrame *pFrame = static_cast<CChildFrame *>(GetParentFrame());
		CCartesianView *pView = pFrame->GetCartesianView();
		if (pView->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
			return TRUE;
	}

	return CFormView::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}
