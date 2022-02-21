#if !defined(AFX_MANUALCONTROLVIEW_H__469FDE22_5C9E_11D8_86C3_0008A15E291C__INCLUDED_)
#define AFX_MANUALCONTROLVIEW_H__469FDE22_5C9E_11D8_86C3_0008A15E291C__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// ManualControlView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CManualControlView form view

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

class CCNCComms;
class CControllerPath;
class CParamDoc;


class CManualControlView : public CFormView
{
//protected:
public:
	CManualControlView();	// protected constructor used by dynamic creation
protected:
	DECLARE_DYNCREATE(CManualControlView)

// Form Data
public:
	//{{AFX_DATA(CManualControlView)
	enum { IDD = IDD_MANUALCONTROL };

	CComboBox	m_PosNewCombo;
	BOOL		m_bEnableMachine;
	int		m_iRampHoldTime;
	int		m_iRampRate;
	int		m_iRampTime;
	int		m_iRampVelChange;
	int		m_iStepHoldTime;
	int		m_iStepAcc;
	int		m_iStepVelChange;
	int		m_iAccType;
	int		m_nFeedBack;
	int		m_nFeedFoward;
	CString	m_strErrorLast;
	//}}AFX_DATA

	CString	m_strPosServo[NUM_AXIS];
	CString	m_strPosCurr[NUM_AXIS];
	CString	m_strPosError[NUM_AXIS];
	CString	m_strVelCurr[NUM_AXIS];
	CString	m_strPosNew[NUM_AXIS];

	BOOL		m_arbEnable[NUM_AXIS];		// axis enabled
	bool		m_arbAxisPosLocated[NUM_AXIS];	// used to color editboxs

	enum { USE_CURRENT, USE_LOCATION, USE_ENTERED };
	int		m_arnPosNewSource[NUM_AXIS];		// use location value, entered value, or current
	BOOL		m_arbUseLoc[NUM_AXIS];
	bool		m_arbUseEntered[NUM_AXIS];
	double	m_arPosNew[NUM_AXIS];
	// arPosNew sources
	double	m_arPosCurr[NUM_AXIS];
	double	m_arPosLoc[NUM_AXIS];
	double	m_arPosEntered[NUM_AXIS];
	

// Attributes
public:

protected:
	CCNCComms* m_pCNCComms;
	CControllerPath* m_pControllerPath;

	bool m_bRegularUpdate;

	SAxisDataXcng m_ADXcng;

	COLORREF m_clrPosNotLocated;
	CFont m_FontPos;
	CBrush m_brPosNotLocated;

	// Move To Locations
	double m_arLocations[10][NUM_AXIS];



// Operations
public:
	CParamDoc* GetDocument();

	void SetCNCComms(CCNCComms* pCNCComms) { m_pCNCComms = pCNCComms; }
	void SetControllerPath(CControllerPath* pControllerPath) { m_pControllerPath = pControllerPath; }

	void OnUpdateParam(CAxPar axPar);
	void OnUpdateState();



protected:
	bool SendMonitorMessage(int iMsg, int wParam = 0, long lParam = 0);
	void OnUpdatePosNew();



// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CManualControlView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CManualControlView();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif


	// Generated message map functions
	//{{AFX_MSG(CManualControlView)
	afx_msg void OnButtonstopxy();
	afx_msg void OnButtonstopz();
	afx_msg void OnButtonSetParam();
	afx_msg void OnButtonEnable();
	afx_msg void OnButtonEnableX();
	afx_msg void OnButtonEnableY();
	afx_msg void OnButtonEnableZ();
	afx_msg void OnKillfocusEditAcc();
	afx_msg void OnButtonMatchTracker();
	afx_msg void OnButtonMoveToPos();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnSelendokComboPosNew();
	afx_msg void OnSelendokComboControlType();
	afx_msg void OnDestroy();
	afx_msg void OnButtonProbeSampler();
	//}}AFX_MSG
	afx_msg void OnButtonChangeVel(UINT nID);
	afx_msg void OnButtonUseLoc(UINT nID);
	afx_msg void OnChangeEditPosNew(UINT nID);
	afx_msg void OnKillfocusEditPosNew(UINT nID);

	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in ManualControlView.cpp
inline CParamDoc* CManualControlView::GetDocument()
	{ return (CParamDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MANUALCONTROLVIEW_H__469FDE22_5C9E_11D8_86C3_0008A15E291C__INCLUDED_)
