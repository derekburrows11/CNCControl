// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINFRM_H__4E7108A8_678C_11D4_8C1E_C12B6D9C0F2F__INCLUDED_)
#define AFX_MAINFRM_H__4E7108A8_678C_11D4_8C1E_C12B6D9C0F2F__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

//#include "DiagnoseDlg.h"
#include "PlayerBar.h"
#include "PathPositionDlg.h"


#include "..\Graph\Plot.h"

class CDiagnoseDlg;
class CManualControlView;
class CMachineStatusView;
class CProbeSampleView;

class CMainFrame : public CMDIFrameWnd
{
	DECLARE_DYNAMIC(CMainFrame)
protected:
enum {				// Plot Message Status
	PMS_NONE = 0,
	PMS_INITIATING,
	PMS_SENDING_DATA,
	};
public:
	CMainFrame();

// Attributes
public:

protected:
	int m_iPlotMsgStatus;
	int m_iNumGraphInst;
	HWND m_hGraphWnd;

	ATOM m_aApp, m_aTopic;

	bool m_bDDEPOKEfRelease;
	HGLOBAL m_hDataPOKE;

	// label for info storage
	char* m_szStorageLabel;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
public:
	void CloseDiagnoseBox();
	virtual ~CMainFrame();

	void SetStatusPosition(const CVect2& ptPos);

	int SendPlot(const double* arX, const double* arY, int numPoints, int plotStyle = GPS_LINE);
	int SendPlot(const float* arX, const float* arY, int numPoints, int plotStyle = GPS_LINE);
	int SendPlot(const CVect2* arPt, int numPoints, int plotStyle = GPS_LINE);
	int SendPlot(const CVect2f* arPt, int numPoints, int plotStyle = GPS_LINE);

	int SendPlotCommand(int command, int p1 = 0, int p2 = 0);

protected:
	int CheckGraphRunning();
	HGLOBAL AllocateMemToSendPlot(SGraphPlotInfo& gpi);
	int SendGraphInfo(HGLOBAL hData);


public:

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	CDiagnoseDlg*       m_pDiagnoseDlg;
	CManualControlView* m_pManualControlView;
	CMachineStatusView* m_pMachineStatusView;
	CProbeSampleView*   m_pProbeSampleView;

protected:  // control bar embedded members
	CStatusBar  m_wndStatusBar;
	CToolBar    m_wndStandardBar;
	CToolBar    m_wndPathDrawBar;
	CToolBar    m_wndMotionPlotBar;
	CToolBar    m_wndViewBar;
	CToolBar    m_wndViewDirBar;

	CPathPositionDlg m_wndPathPosDlg;		// to set displayed path position

public:
	CPlayerBar  m_wndPlayerBar;		// to play animations

protected:
	CView* m_pActivePathView;		// maybe could just just GetActiveView()?


// Generated message map functions
protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnMachineDiagnoseBox();
	afx_msg void OnClose();
	afx_msg void OnMachineProperties();
	afx_msg void OnPathSegmentTracking();
	afx_msg void OnMachineManualCont();
	afx_msg void OnManualControlViewClosed();
	afx_msg void OnDestroy();
	afx_msg void OnMachineStatus();
	afx_msg void OnMachineSetTool();
	afx_msg void OnMachineProbeSample();
	afx_msg void OnProbeSampleViewClosed();
	afx_msg void OnOptionsSettings();
	//}}AFX_MSG
	afx_msg void OnMachineManualCont2();
	afx_msg void OnViewToolBar(UINT nID);
	afx_msg void OnUpdateViewToolBar(CCmdUI* pCmdUI);

	afx_msg LRESULT OnMemValue(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSetPosIndicator(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnUpdateParam(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSetActivePathView(WPARAM wParam, LPARAM lParam);

//	afx_msg LRESULT OnDDEInitiate(WPARAM wParam, LPARAM lParam);
//	afx_msg LRESULT OnDDEExecute(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnDDETerminate(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnDDEAcknowledge(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__4E7108A8_678C_11D4_8C1E_C12B6D9C0F2F__INCLUDED_)
