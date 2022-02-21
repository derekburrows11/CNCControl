// ControllerPath.h: interface for the CControllerPath class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CONTROLLERPATH_H__B875B280_9FE4_11D7_86C3_CEEBEBC09725__INCLUDED_)
#define AFX_CONTROLLERPATH_H__B875B280_9FE4_11D7_86C3_CEEBEBC09725__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


//#include "ControllerTracker.h"
#include "PathSpeed.h"
#include "PathTimeStep.h"
#include "PathDoc.h"
#include "CNCComms.h"

class CParamDoc;


class CControllerPath  
{
public:
	CControllerPath();
	virtual ~CControllerPath();

	// functions
	void Init();
//	CSegAccelFIFO* GetSegmentAccelBuffer() { return &m_SegAccelFIFO; }
	void SetCNCComms(CCNCComms* pComms) { m_pComms = pComms; }
	void SetParamDoc(CParamDoc* pDoc) { m_pParamDoc = pDoc; }

	bool SetPathDoc(CPathDoc* pPathDoc);
	CPathDoc* GetPathDoc() { return m_pPathDoc; }
	bool GotPath() { return m_pPathDoc != NULL; }
	void ResetPath();
	void StopSendingPath();
	void SendPath();			// continuly sends path
	void SendMorePath();		// called to request more path data
	bool IsSendingPath() { return m_bSendingPath; }
	void OnPathFinished();

	bool MoveToPositionRelBase(CVector& vtPosNew);
	bool MoveRelative(CVector& vtPosRel);
	bool MoveLine(CVector& vtStart, CVector& vtEnd);

//	void ShowStatus(bool bShow)	;
//	bool IsStatusVisible();


protected:
	void StartSendingPath();
	void FillSegAccelBuffer();
	void FindAllLimits();		// for checking limit calc's
	void EmptySegAccelBufferTo(int numRemain);

	bool SendMonitorMessage(int iMsg, int wParam = 0, long lParam = 0) { if (m_pComms != NULL) return m_pComms->SendMonitorMessage(iMsg, wParam, lParam); return false; }

	// data
protected:
//	CMachineStatusView m_MachineStatusView;
	CCNCComms* m_pComms;
	CControllerTracker m_ContTracker;	// tracks actual machine path

	CPathSpeed m_PathSpeed;			// calcutates speed limits along path
	CPathTimeStep m_PathStepper;	// calcutates steps using speed limits
	CLimitList m_LimitList;			// tempory section resulting speed limits
	CSegAccelFIFO m_SegAccelFIFO;	// ouput buffer for accel segments

	bool m_bSendingPath;

	CPathDoc* m_pPathDoc;
	CParamDoc* m_pParamDoc;



public:
	CPathDoc m_ManualPath;

	bool m_bTestRun;					// doesn't send data to machine

};

#endif // !defined(AFX_CONTROLLERPATH_H__B875B280_9FE4_11D7_86C3_CEEBEBC09725__INCLUDED_)
