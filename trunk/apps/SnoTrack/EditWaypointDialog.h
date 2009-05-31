#pragma once
#include "resource.h"
#include "Waypoint.h"

class Projection;

// CEditWaypointDialog dialog
class CEditWaypointDialog : public CDialog
{
	Waypoint m_waypoint;
	Projection *m_proj;

	DECLARE_DYNAMIC(CEditWaypointDialog)

public:
	CEditWaypointDialog(const Waypoint &waypoint, Projection *proj, CWnd* pParent = NULL);   // standard constructor
	virtual ~CEditWaypointDialog();

	virtual BOOL OnInitDialog();

	Waypoint GetWaypoint() const { return m_waypoint; }

// Dialog Data
	enum { IDD = IDD_EDIT_WAYPOINT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnOK();

	DECLARE_MESSAGE_MAP()
};
