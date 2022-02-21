#if !defined(AFX_CONTROLSTATUSVIEW_H__8D6A8060_B79B_11D7_86C3_F413A6444625__INCLUDED_)
#define AFX_CONTROLSTATUSVIEW_H__8D6A8060_B79B_11D7_86C3_F413A6444625__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// MachineStatusView.h : header file
//

#include "PathDataObjects.h"
class CParamDoc;
class CMachineState;

/////////////////////////////////////////////////////////////////////////////
// CMachineStatusView form view

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif




class CMachineStatusView : public CFormView
{
// Construction
//protected:
public:
	CMachineStatusView();	// protected constructor used by dynamic creation
protected:
	DECLARE_DYNCREATE(CMachineStatusView)




// Operations
	CParamDoc* GetDocument();

	void UpdateLimitBufferStatus(SContBufferSpan& span);
	void UpdateAccSegBufferStatus(SContBufferSpan& span);
	void UpdateMachineBufferStatus(SContBufferSpan& span);
	void UpdateMachineState(CMachineState& state);


// Form Data
protected:
	//{{AFX_DATA(CMachineStatusView)
	enum { IDD = IDD_CONTROLSTATUS };

	double	m_AccSeg_TI;
	double	m_AccSeg_TF;
	int		m_iAccSeg_PSI;
	int		m_iAccSeg_PSF;
	int		m_iAccSeg_ASI;
	int		m_iAccSeg_ASF;
	double	m_Limit_TI;
	double	m_Limit_TF;
	int		m_iLimit_PSI;
	int		m_iLimit_PSF;
	double	m_Machine_TI;
	double	m_Machine_TF;
	int		m_iMachine_ASI;
	int		m_iMachine_ASF;
	CString	m_strPos;
	CString	m_strVel;
	CString	m_strPosError;
	CString	m_strPosTrack;
	CString	m_strVelError;
	CString	m_strVelTrack;
	double	m_PathTime;
	int		m_iPathStep;
	//}}AFX_DATA


// Attributes
public:

	double	m_Path_TI;
	double	m_Path_TF;
	int		m_iPath_PSI;
	int		m_iPath_PSF;



protected:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMachineStatusView)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CMachineStatusView();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif


	// Generated message map functions
	//{{AFX_MSG(CMachineStatusView)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in MachineStatusView.cpp
inline CParamDoc* CMachineStatusView::GetDocument()
	{ return (CParamDoc*)m_pDocument; }
#endif

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONTROLSTATUSVIEW_H__8D6A8060_B79B_11D7_86C3_F413A6444625__INCLUDED_)
