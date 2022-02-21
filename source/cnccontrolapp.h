// CNCControlApp.h : header file for class CCNCControlApp
//

#if !defined(AFX_CNCCONTROLAPP_H)
#define AFX_CNCCONTROLAPP_H

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


//#include "ParamDoc.h"
class CParamDoc;
class CLogDoc;

#include "CNCComms.h"	// Added by ClassView
#include "ControllerPath.h"

/////////////////////////////////////////////////////////////////////////////
// CCNCControlApp:
// See CNCControl.cpp for the implementation of this class
//

class CCNCControlApp : public CWinApp
{
	DECLARE_DYNAMIC(CCNCControlApp)
public:
	CCNCControlApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCNCControlApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	virtual LRESULT ProcessWndProcException(CException* e, const MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
public:
	// data
	CCNCComms m_CNCComms;
	CParamDoc* m_pParamDoc;
	CLogDoc* m_pLogDoc;
	CControllerPath m_ControllerPath;	// tracks and records the currently transmitted path



	CDocTemplate* m_pParamDocTemplate;
	CDocTemplate* m_pPathDocTemplate;
	CDocTemplate* m_pPathSpeedDocTemplate;
//	CDocTemplate* m_pPathAnimateDocTemplate;
	CDocTemplate* m_pLogDocTemplate;
	
	// functions
	void CreateParamView(int nViewType);
	void SetSimulateResponse(bool bSim);
	bool SetControllerPath(CPathDoc* pPathDoc) { return m_ControllerPath.SetPathDoc(pPathDoc); }
	bool IsSendingPath() { return m_ControllerPath.IsSendingPath(); }

	void PathSendStart() { OnPathSendStart(); }		// public access to command functions


protected:
	// data
	bool m_bSimulateControllerResponse;		// modifies recieved comms as if response for controller
	bool m_bSendPktStep;

	CFont m_fontText;

	// functions
protected:
	//{{AFX_MSG(CCNCControlApp)
	afx_msg void OnAppAbout();
	afx_msg void OnMachineOpenComms();
	afx_msg void OnMachineCloseComms();
	afx_msg void OnMachineOpenCloseComms();
	afx_msg void OnUpdateMachineOpenCloseComms(CCmdUI* pCmdUI);
	afx_msg void OnOptionsPortSelect();
	afx_msg void OnOptionsPortConfig();
	afx_msg void OnViewParamAxis();
	afx_msg void OnViewParamGeneral();
	afx_msg void OnViewParamAll();
	afx_msg void OnFileOpen();
	afx_msg void OnMachineSimulateResponse();
	afx_msg void OnUpdateMachineSimulateResponse(CCmdUI* pCmdUI);
	afx_msg void OnPathSendStart();
	afx_msg void OnPathSendStop();
	afx_msg void OnMachineShowCommsData();
	afx_msg void OnUpdateMachineShowCommsData(CCmdUI* pCmdUI);
	afx_msg void OnMachineLogCommsData();
	afx_msg void OnUpdateMachineLogCommsData(CCmdUI* pCmdUI);
	afx_msg void OnMachineSendBytes();
	afx_msg void OnPathSendReset();
	afx_msg void OnUpdatePathSendStart(CCmdUI* pCmdUI);
	afx_msg void OnUpdatePathSendReset(CCmdUI* pCmdUI);
	afx_msg void OnSendPacket();
	afx_msg void OnSendPktStep();
	afx_msg void OnUpdateSendPktStep(CCmdUI* pCmdUI);
	afx_msg void OnUpdatePathSendStop(CCmdUI* pCmdUI);
	afx_msg void OnFileNew();
	afx_msg void OnViewLog();
	afx_msg void OnUpdateViewLog(CCmdUI* pCmdUI);
	//}}AFX_MSG
	afx_msg void OnCommNotify(WPARAM wType, LPARAM lInfo);
	afx_msg void OnUpdateParam(WPARAM axpar, LPARAM lParam);
	afx_msg void OnMemValue(WPARAM, LPARAM);
	afx_msg void OnRegValue(WPARAM, LPARAM);
	afx_msg void OnLogEvent(WPARAM str, LPARAM);

	void RegDocTemplates();
	void LoadConfigSettings();
	void SaveConfigSettings();


	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////


#endif	// !defined(AFX_CNCCONTROLAPP_H)
