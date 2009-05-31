#pragma once
#include "resource.h"
#include "Lift.h"

class Projection;

// CEditLiftDialog dialog
class CEditLiftDialog : public CDialog
{
	Lift m_lift;
	Projection *m_proj;

	DECLARE_DYNAMIC(CEditLiftDialog)

public:
	CEditLiftDialog(const Lift &lift, Projection *proj, CWnd* pParent = NULL);   // standard constructor
	virtual ~CEditLiftDialog();

	virtual BOOL OnInitDialog();

	Lift GetLift() const { return m_lift; }

// Dialog Data
	enum { IDD = IDD_EDIT_LIFT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	virtual void OnOK();
};
